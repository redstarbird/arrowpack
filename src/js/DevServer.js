const http = require('http');
const fs = require("fs");
const path = require('path');
const WebSocket = require("ws");
const url = require('url');

let wss;
const connections = new Map(); // Creates a map to store connections and corresponding pages
const cwd = process.cwd();

// Starts the dev server
function StartServer(Settings) {
    const ScriptInjectData = "<script type=\"module\">" + fs.readFileSync(path.join(__dirname, "ClientDevServer.js")) + "</script>" // Websocket reloading script for the client

    const server = http.createServer( // Creates the HTTP server to serve the webpages
        (req, res) => {
            const parsedURL = url.parse(req.url);
            var filePath = path.join(cwd, Settings.getValue("exit"), parsedURL.pathname) // Find the path to the file`
            if (parsedURL.query) { filePath += path.sep; } // Adds support for query strings

            var fileExtension = path.extname(filePath); // Get the file extension
            var contentType;

            switch (fileExtension) { // Find the content header for the requested file
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

            var redirect = false;
            if (fileExtension === "") {
                if (!req.url.endsWith("/")) {
                    redirect = true;
                }
                filePath = path.join(filePath, "index.html");
            }

            if (redirect) { // Redirect to make sure the path ends with a trailing foward slash
                res.writeHead(302, { "Location": req.url + "/" });
                res.end();
            } else {
                fs.readFile(filePath, (err, data) => {
                    if (err) {
                        res.writeHead(404, { 'Content-Type': 'text/plain' });
                        res.end('404 Not Found\n');
                    } else {

                        if (contentType === "text/html") {
                            var InsertLocation = data.indexOf("</body>"); // Finds the end of the body section
                            data = data.slice(0, InsertLocation) + ScriptInjectData + data.slice(InsertLocation); // Insert the websocket reloading script
                        }

                        res.writeHead(200, { 'Content-Type': contentType, "Cache-Control": "no-store, no-cache", "Expires": "0" }); // Send http response
                        res.end(data);
                    }
                });
            }
        }
    );
    wss = new WebSocket.Server({ server });

    wss.on("connection", (ws) => {
        console.log("Connection established");
        ws.once("message", (message) => {
            message = path.join(Settings.getValue("entry"), message.toString()); // Get file path client is on

            connections.set(ws, message); // Store connection with connected path
        }); ws.on("close", () => {
            console.log("Connection closed");
            connections.delete(ws); // Delete websocket from map
        });
    });

    server.listen(Settings.getValue("devPort"), () => {
        console.log("Started dev server at port " + Settings.getValue("devPort"));
    });
}

// Sends the updated page to any clients on said page
function SendUpdatedPage(filePath, Settings) {
    console.log("Sending updated page...");
    const OpenedFiles = new Map();
    for (const [key, value] of connections.entries()) { // Loop through connections
        for (i = 0; i < filePath.length; i++) { // Check each file path
            if (filePath[i].startsWith("ARROWPACK_TEMP_PREPROCESS_DIR")) { // Change temp paths to exit paths
                filePath[i] = Settings.getValue("entry") + filePath[i].slice(30);
            }

            if (filePath[i] === value) { // Check if the connection is to the given path
                if (!OpenedFiles.has(filePath[i])) {
                    console.log(filePath[i]);
                    var FileContent = fs.readFileSync(Settings.getValue("exit") + filePath[i].slice(Settings.getValue("entry").length), "utf8");
                    OpenedFiles.set(filePath[i], FileContent);
                }
                key.send(OpenedFiles.get(filePath[i])); // Send the client the new page
            }
        }
    }
}
module.exports = { StartServer: StartServer, SendUpdatedPage: SendUpdatedPage };