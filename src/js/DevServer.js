const http = require('http');
const fs = require("fs");
const path = require('path');
const WebSocket = require("ws");
const url = require('url');

let wss;
const connections = new Map();
const cwd = process.cwd();

function StartServer(Settings) {
    const ScriptInjectData = "<script type=\"module\">" + fs.readFileSync(path.join(__dirname, "ClientDevServer.js")) + "</script>"
    const server = http.createServer(
        (req, res) => {
            const parsedURL = url.parse(req.url);
            var filePath = path.join(cwd, Settings.getValue("exit"), parsedURL.pathname)
            if (parsedURL.query) { filePath += path.sep; }

            var fileExtension = path.extname(filePath);
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
            var redirect = false;
            if (fileExtension === "") {
                if (!req.url.endsWith("/")) {
                    redirect = true;
                }
                filePath = path.join(filePath, "index.html");
            }

            if (redirect) {
                res.writeHead(302, { "Location": req.url + "/" });
                res.end();
            } else {
                fs.readFile(filePath, (err, data) => {
                    if (err) {
                        res.writeHead(404, { 'Content-Type': 'text/plain' });
                        res.end('404 Not Found\n');
                    } else {

                        if (contentType === "text/html") {
                            var InsertLocation = data.indexOf("</body>");
                            data = data.slice(0, InsertLocation) + ScriptInjectData + data.slice(InsertLocation);
                        }

                        res.writeHead(200, { 'Content-Type': contentType, "Cache-Control": "no-store, no-cache", "Expires": "0" });
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
            message = path.join(Settings.getValue("entry"), message.toString());

            connections.set(ws, message);
        }); ws.on("close", () => {
            console.log("Connection closed");
            connections.delete(ws);
        });
    });

    server.listen(Settings.getValue("devPort"), () => {
        console.log("Started dev server at port " + Settings.getValue("devPort"));
    });
}

function SendUpdatedPage(filePath, Settings) {
    console.log("Sending updated page...");
    const OpenedFiles = new Map();
    for (const [key, value] of connections.entries()) {
        for (i = 0; i < filePath.length; i++) {
            if (filePath[i].startsWith("ARROWPACK_TEMP_PREPROCESS_DIR")) {
                filePath[i] = Settings.getValue("entry") + filePath[i].slice(30);
            }

            if (filePath[i] === value) {
                if (!OpenedFiles.has(filePath[i])) {
                    console.log(filePath[i]);
                    var FileContent = fs.readFileSync(Settings.getValue("exit") + filePath[i].slice(Settings.getValue("entry").length), "utf8");
                    OpenedFiles.set(filePath[i], FileContent);
                }
                key.send(OpenedFiles.get(filePath[i]));
            }
        }
    }
}
module.exports = { StartServer: StartServer, SendUpdatedPage: SendUpdatedPage };