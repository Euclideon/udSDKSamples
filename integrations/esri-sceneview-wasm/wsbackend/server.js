const express = require('express');
const SocketServer = require('ws').Server;
const port = 4006;

// Create express server object and listen to port 4006
const server = express().listen(port, () => {
  console.log(`Server Listening on port ${port}...`);
});

// Hand over express object to Socket Server to turn on web socket.
const wss = new SocketServer({ server });

wss.on('connection', ws => {

  // When a client connected
  console.log('Client connected');


  const sendNowTime = setInterval(() => {
    ws.send(JSON.stringify({
      currentTime: new Date()
    }))
  }, 990);


  ws.on('message', data => {
    // data is an object received from the client
    ws.send(data);
  })

  // When Web socket connection is closed
  ws.on('close', () => {
    console.log('Connection closed')
  });
});
