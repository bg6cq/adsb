var PORT = 33088;
var HOST = '127.0.0.1';

var dgram = require('dgram');
var udpserver = dgram.createSocket('udp4');

udpserver.on('listening', function () {
    	var address = udpserver.address();
    	console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

udpserver.bind(PORT, HOST);

function ab2str(buf) {
  return String.fromCharCode.apply(null, new Uint8Array(buf));
}

udpserver.on('message', function (msg, remote) {
	// console.log(remote.address + ':' + remote.port +' - ' + msg);
	// console.log(".");
	var txt = ab2str(msg);
        console.log(txt);
        var obj = eval ("(" + txt + ")");
        console.log(obj.aid);
	for ( sock in usersockets ) {
		if( lon1[sock] <= obj.lon && obj.lon <=lon2[sock] 
		  &&lat1[sock] <= obj.lat && obj.lat <=lat2[sock] ) {
			console.log("sending to user "+userindex[sock]);
			usersockets[sock].emit('ADSB', msg); 
		} else
			console.log("do not sending to user "+userindex[sock]);
	}
    	// io.sockets.emit('ADSB', msg);
});

var express = require('express'); // Get the module
var app = express(); // Create express by calling the prototype in var express
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.use(express.static('html'));

app.get('/', function(req, res){
	res.sendFile(__dirname + 'html' + '/index.html');
});

var usersockets = {};
var usercount = 0;
var userindex = {};
var lon1 = {};
var lon2 = {};
var lat1 = {};
var lat2 = {};
io.on('connection', function(socket){
	usersockets[socket]=socket;
	usercount++;
	userindex[socket]=usercount;
	lon1[socket] = 0;
	lon2[socket] = 180;
	lat1[socket] = 0;
	lat2[socket] = 90;
	console.log('websocket user '+usercount+' connected');
	socket.on('disconnect', function(){
		console.log('websocket user '+userindex[socket]+' disconnected');
		delete  usersockets[socket];
		delete  userindex[socket];
		delete lon1[socket];
		delete lon2[socket];
		delete lat1[socket];
		delete lat2[socket];
	});
	socket.on('viewchange', function(data){
		console.log('websocket user '+userindex[socket]+' viewchange: '+data.lon1+'-'+data.lon2+'/'+data.lat1+'-'+data.lat2);
		lon1[socket]=data.lon1;
		lon2[socket]=data.lon2;
		lat1[socket]=data.lat1;
		lat2[socket]=data.lat2;
	});
});

http.listen(3000, function(){
	console.log('listening on *:3000');
});
