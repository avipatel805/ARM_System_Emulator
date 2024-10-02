const WebSocket = require('ws');
const net = require('net');

const wsServer = new WebSocket.Server({ port: 8080 });

wsServer.on('connection', function connection(ws) {
	  const client = new net.Socket();
	  
	  client.connect(8888, '127.0.0.1', function() {
		      console.log('Connected to TCP server');
		    });
	  
	  client.on('data', function(data) {
		      console.log('received from TCP server: %s', data);
		      ws.send(data.toString());
		    });
	  
	  ws.on('message', function incoming(message) {
		      console.log('received from WS client: %s', message);
		      client.write(message);
		    });

	  ws.on('close', function() {
		      console.log('WebSocket client disconnected');
		      client.end();
		    });

	  client.on('close', function() {
		      console.log('Connection to TCP server closed');
		      ws.close();
		    });
});

console.log('WebSocket server started on ws://localhost:8080');

