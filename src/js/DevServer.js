const http = require('http');
const fs = require("fs");
const path = require('path');
const WebSocket = require("ws");

let wss;
const connections = new Map();

function StartServer(Settings) {
    const ScriptInjectData = "<script type=\"module\">" + fs.readFileSync("src/js/ClientDevServer.js") + "</script>"
    console.log(ScriptInjectData);
    const server = http.createServer(
        (req, res) => {
            var filePath = path.join(process.argv[1], Settings.getValue("exit"), req.url);
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

            if (fileExtension === "") {
                filePath = path.join(filePath, "index.html");
            }

            // console.log(filePath);
            fs.readFile(filePath, (err, data) => {
                if (err) {
                    res.writeHead(404, { 'Content-Type': 'text/plain' });
                    res.end('404 Not Found\n');
                } else {

                    if (contentType === "text/html") {
                        var InsertLocation = data.indexOf("</body>");
                        data = data.slice(0, InsertLocation) + ScriptInjectData + data.slice(InsertLocation);
                        console.log(InsertLocation);
                    }

                    res.writeHead(200, { 'Content-Type': contentType, "Cache-Control": "no-store, no-cache", "Expires": "0" });
                    res.end(data);
                }
            });
        }
    );
    wss = new WebSocket.Server({ server });



    wss.on("connection", (ws) => {
        console.log("Connection established");
        ws.once("message", (message) => {
            message = path.join(Settings.getValue("entry"), message.toString());
            console.log("Received message: " + message);

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
        console.log("Key: " + key + " Value: " + value);
        for (i = 0; i < filePath.length; i++) {
            console.log("File: " + filePath[i] + " value: " + value);
            if (filePath[i] === value) {
                if (!OpenedFiles.has(filePath[i])) {
                    console.log(filePath[i]);
                    var FileContent = fs.readFileSync(Settings.getValue("exit") + filePath[i].slice(Settings.getValue("entry").length), "utf8");
                    OpenedFiles.set(filePath[i], FileContent);
                }
                console.log("Sending file " + filePath[i]);
                key.send(OpenedFiles.get(filePath[i]));
            }
        }
    }
}
module.exports = { StartServer: StartServer, SendUpdatedPage: SendUpdatedPage };