var PORT = 33088;
var HOST = '127.0.0.1';

var dgram = require('dgram');
var udpserver = dgram.createSocket('udp4');

udpserver.on('listening', function () {
    	var address = udpserver.address();
    	console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

udpserver.bind(PORT, HOST);

udpserver.on('message', function (msg, remote) {
	// console.log(remote.address + ':' + remote.port +' - ' + msg);
	// console.log(".");
    	io.sockets.emit('ADSB', msg);
});

var express = require('express'); // Get the module
var app = express(); // Create express by calling the prototype in var express
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.use(express.static('html'));

app.get('/', function(req, res){
	res.sendFile(__dirname + 'html' + '/index.html');
});

io.on('connection', function(socket){
	console.log('websocket user connected');
	socket.on('disconnect', function(){
		console.log('websocket user disconnected');
	});
});

http.listen(3000, function(){
	console.log('listening on *:3000');
});
