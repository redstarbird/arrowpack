let dirname = window.location.pathname;
if (dirname.endsWith("/")) {
    dirname += "index.html";
} else { dirname += "/index.html"; }
console.log(dirname);
// dirname = dirname.split('/').pop();
function ReplaceDocument(NewHead, NewBody) {
    document.head.innerHTML = NewHead;
    document.body.innerHTML = NewBody;
}

function extractHead(html) {
    // Match the head element and its contents
    const headMatch = /<head[^>]*>([\s\S]*?)<\/head>/i.exec(html);
    // If a match was found, return the head content
    if (headMatch && headMatch.length > 1) {
        return headMatch[1];
    }
    // If no match was found, return an empty string
    return "";
}

function extractBody(html) {
    const bodyStart = html.indexOf("<body>"); // add 6 to exclude the <body> tag
    const bodyEnd = html.indexOf("</body>");
    return html.substring(bodyStart, bodyEnd);
}

const ws = new WebSocket("ws://" + location.hostname + ":8080");

ws.onopen = (event) => {
    console.log("opened websocket, sending data " + dirname);
    ws.send(dirname);
};

ws.onmessage = (event) => {
    console.log(event.data);
    if (event.data.includes("script")) { window.location.reload(); } else {
        ReplaceDocument(extractHead(event.data), extractBody(event.data));
    }
}
window.addEventListener('beforeunload', function () {
    ws.close();
});
