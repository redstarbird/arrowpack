const http = require('http');
const fs = require("fs");
const path = require('path');
const WebSocket = require("ws");
const url = require('url');
const EventEmitter = require("events");
const { ArrowDeserialize, ArrowSerialize } = require('../util/serialize.cjs');
const Config = require("../config.js");
const chokidar = require('chokidar');
const chalk = require("chalk");

class DevServer extends EventEmitter {
    constructor({ config, nativeModule, dependencyGraphPtr }) {
        super();
        this.config = config;
        this.nativeModule = nativeModule;
        this.dependencyGraphPtr = dependencyGraphPtr;

        this.cwd = process.cwd();
        // Map between connected clients and corresponding pages
        this.connections = new Map();
        // WebSocket server
        this.wss = null;
        // File system watcher
        this.watcher = chokidar.watch(this.config.getValue("entry"), { usePolling: true, ignoreInitial: true, persistent: true });
        // HTTP static websever
        this.webserver = null;
    }

    /**
     * Starts the arrowpack dev server including the HTTP webserver, WebSocket server, and the file watcher.
     * 
     * This emits the `listening` event with the port the webserver is running on.
     * @returns {Promise<DevServer>}
     */
    async start() {

        // Create injection script HTML element
        const ScriptInjectData = "<script type=\"module\">" + fs.readFileSync(path.join(__dirname, "ClientDevServer.js")) +
            "</script>";

        // Start static HTTP webserver
        this.webserver = http.createServer(
            (req, res) => { this._HandleRequest(req, res, ScriptInjectData) }
        );

        // Start websocket server for refreshing rebuilt pages
        this.wss = new WebSocket.Server({ server: this.webserver });

        this.wss.on("connection", (ws) => {
            console.log("Connection established");

            // On first message, store the client and the page that they are on
            ws.once("message", (message) => {
                // Get file path that client is on
                message = path.join(this.config.getValue("entry"),
                    message.toString());

                this.connections.set(ws, message);
            });

            // Delete connection from map on page close
            ws.on("close", () => {
                console.log("Connection closed");
                this.connections.delete(ws);
            });
        });

        const HandleFileChange = (filepath) => {
            this.rebuild([filepath]);
        }

        /* 
        * Start file system watcher. This calls the native `RebuildFiles` function to rebuild the specific file(s) that have changed
        */
        this.watcher.on("change", HandleFileChange);
        this.watcher.on("add", HandleFileChange)

        /* Start webserver listening on port */
        return new Promise((resolve, reject) => {
            this.webserver.once("error", reject);
            this.webserver.listen(this.config.getValue("devPort"), () => {
                console.log("Started dev server at port " + this.config.getValue("devPort"));

                this.emit("listening", this.config.getValue("devPort"))

                resolve(this);
            });
        })

    }

    /**
     * Stops and cleans up the HTTP webserver, WebSocket server, and file watcher.
     * The server can be restarted after this using `DevServer.start()`.
     * 
     * This emits a `close` event.
     */
    async stop() {
        // Cleanup file system watcher
        if (this.watcher) {
            await this.watcher.close();
            this.watcher = null;
        }

        // Cleanup websocket server
        if (this.wss) {
            for (const connection of this.connections.keys()) {
                connection.terminate();
            }
            this.connections.clear()
            await new Promise((resolve) => this.wss.close(resolve));
            this.wss = null;
        }

        // Cleanup HTTP webserver
        if (this.webserver) {
            await new Promise((resolve) => this.webserver.close(resolve));
            this.webserver = null;
        }

        this.emit("close");
    }

    /**
     * Rebuilds the files at the provided paths as well as their dependents, any clients connected on a modified page will be sent the updated page.
     * @param {string[]} filePaths Array of relative file paths to rebuild
     */
    rebuild(filePaths) {
        if (!Array.isArray(filePaths) || filePaths.length == 0) {
            return []
        }

        for (const FilePath of filePaths) {
            console.log("File " + FilePath + " has changed, rebuilding...");
        }

        // Store start time to track bundling time
        var StartTime = performance.now();

        // Convert paths into a single string to send to C side
        const serialisedPaths = ArrowSerialize(filePaths);

        /*
         * Calls the native C RebuildFiles function from Main.c
         * Called via native module abstraction object and provided the graph pointer, the path of the changed file, and the number of
         * files (currently always 1). This returns a serialised array of modified files so that
         * clients connected to those files can be refreshed
        */
        var RebuiltFiles = this.nativeModule.RebuildFiles(this.dependencyGraphPtr, serialisedPaths, 1);

        // Deserialise string of changed files
        RebuiltFiles = ArrowDeserialize(RebuiltFiles);

        // Send the updated pages to clients
        this._SendUpdatedPages(RebuiltFiles);

        this.emit("rebuild");

        console.log(chalk.magentaBright("\n\nBundling files completed in " +
            (performance.now() - StartTime) / 1000 + " seconds\n\n"));
    }

    /**
     * 
     * @param {http.IncomingMessage} req client request
     * @param {http.ServerResponse<http.IncomingMessage>} res server response
     * @param {string} ScriptInjectData HTML element containing the client-side websocket script
     */
    _HandleRequest(req, res, ScriptInjectData) {
        // Find the path to the file from the request
        const parsedURL = url.parse(req.url);
        var filePath = path.join(this.cwd, this.config.getValue("exit"),
            parsedURL.pathname)

        // Handle query string
        if (parsedURL.query) {
            filePath += path.sep;
        }

        var fileExtension = path.extname(filePath);

        // Find the content-type header for the requested file
        var contentType;
        switch (fileExtension) {
            case ".js":
                contentType = "text/javascript";
                break;
            case ".css":
                contentType = "text/css";
                break;
            case ".json":
                contentType = "application/json";
                break;
            case ".png":
                contentType = "image/png";
                break;
            case ".jpg":
                contentType = "image/jpg";
                break;
            case ".gif":
                contentType = "image/gif";
                break;
            case ".svg":
                contentType = "image/svg+xml"
                break;
            case ".webp":
                contentType = "image/webp";
                break;
            case ".html", ".htm", "":
                contentType = "text/html";
                break;
            default:
                contentType = "text/plain";
        }

        /*
         If the there is no file extension, assume the file is a HTML file
         */
        var redirect = false;
        if (fileExtension === "") {
            if (!req.url.endsWith("/")) {
                redirect = true;
            }
            filePath = path.join(filePath, "index.html");
        }

        // Redirect if necessary to make sure the path ends with a trailing foward slash
        if (redirect) {
            res.writeHead(302, { "Location": req.url + "/" });
            res.end();
        }
        else {
            fs.readFile(filePath, (err, data) => {
                // Error should be 404
                if (err) {
                    res.writeHead(404, { 'Content-Type': 'text/plain' });
                    res.end('404 Not Found\n');
                }
                else {

                    // Insert the websocket reloading script if the file is a HTML file
                    if (contentType === "text/html") {
                        var InsertLocation = data.indexOf("</body>");  // Finds the end of the body section
                        data = data.slice(0, InsertLocation) + ScriptInjectData +
                            data.slice(InsertLocation);  // Insert the websocket reloading script
                    }

                    // Send HTTP response
                    res.writeHead(200, {
                        'Content-Type': contentType,
                        "Cache-Control": "no-store, no-cache",
                        "Expires": "0"
                    });
                    res.end(data);
                }
            });
        }
    }

    /**
     * Sends the new page to every client connected to any of the provided file paths
     * @param {string[]} filePaths Relative entry paths of modified files
     */
    _SendUpdatedPages(filePaths) {
        console.log("Sending updated page...");

        // Cache file contents for opened files
        const OpenedFiles = new Map();

        // Iterate through connections
        for (const [connection, clientPath] of this.connections.entries()) {
            // Iterate through paths of modified files
            for (let i = 0; i < filePaths.length; i++) {
                // Change temp paths to exit paths by slicing preprocess directory name
                if (filePaths[i].startsWith("ARROWPACK_TEMP_PREPROCESS_DIR")) {
                    filePaths[i] = this.config.getValue("entry") + filePaths[i].slice(30);
                }

                /*
                * If the connection is to the given path, get the file content and send it to the client
                */
                if (filePaths[i] === clientPath) {

                    // Once a file's content is read, add its content to the map to cache it
                    if (!OpenedFiles.has(filePaths[i])) {
                        var FileContent = fs.readFileSync(
                            this.config.getValue("exit") + filePaths[i].slice(this.config.getValue("entry").length), "utf8");

                        OpenedFiles.set(filePaths[i], FileContent);
                    }

                    // Send the client the new page over the websocket
                    connection.send(OpenedFiles.get(filePaths[i]));
                }
            }
        }
    }
}

module.exports = DevServer;