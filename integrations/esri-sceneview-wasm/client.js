const ws = new WebSocket('ws://localhost:4006', ['json', 'xml']);

ws.addEventListener('open', () => {
  const data = { message: 'Hello from the client!' }
  const json = JSON.stringify(data);
  ws.send(json);
});

ws.addEventListener('message', event => {
  const data = JSON.parse(event.data);
  console.log(data);
});