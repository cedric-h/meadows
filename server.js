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

const server = app.listen(process.env.PORT || 3000);
console.log(`Listening on port ${process.env.PORT || 3000}`)
server.on('upgrade', (...args) => wss.handleUpgrade(...args, connect));

