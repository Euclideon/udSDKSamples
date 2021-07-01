// use web socket url to open a connection to server
let ws = new WebSocket('ws://localhost:4006')

// callback when connection open
ws.onopen = () => {
    console.log('open connection')
}

// callback when connection is closed
ws.onclose = () => {
    console.log('close connection')
}

ws.onmessage = (event) => {
    document.getElementById("display").innerHTML = "<h1>"+JSON.parse(event.data).currentTime+"</h1>";
    console.log(JSON.parse(event.data));
}