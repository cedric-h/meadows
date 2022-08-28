const express = require('express');
const ws = require('ws');

const app = express();

app.use(express.static('pub'))

const wss = new ws.Server({ noServer: true });

function connect(ws) {
  ws.on('message', (data, isBinary) => {
    for (const client of wss.clients)
      if (client !== ws && client.readyState === ws.OPEN)
        client.send(data, { binary: isBinary });
  });
}

<<<<<<< HEAD
const server = app.listen(3000);
server.on('upgrade', (...args) => wss.handleUpgrade(...args, connect));
=======
const server = app.listen(3000);
server.on('upgrade', (...args) => wss.handleUpgrade(...args, connect));
>>>>>>> e3a05a21ad4be03f99add3546fb7a189692961f3
