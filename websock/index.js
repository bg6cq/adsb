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
	var txt = ab2str(msg);
        //console.log(txt);
	var obj = JSON.parse(txt);
	for ( uidx in usersockets ) {
		if( lon1[uidx] <= obj.lon && obj.lon <=lon2[uidx]
		  &&lat1[uidx] <= obj.lat && obj.lat <=lat2[uidx] ) {
			console.log("sending to user "+userindex[uidx]);
			usersockets[uidx].emit('ADSB', obj);
		} else
			console.log("do not sending to user "+userindex[uidx]);
	}
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
	usersockets[socket.id]=socket;
	usercount++;
	userindex[socket.id]=usercount;
	lon1[socket.id] = 0;
	lon2[socket.id] = 180;
	lat1[socket.id] = 0;
	lat2[socket.id] = 90;
	console.log('websocket user '+usercount+' '+socket.id+' connected');
	socket.on('disconnect', function(){
//		(function(){
		console.log('websocket user '+userindex[socket.id]+' '+socket.id+' disconnected');
		delete  usersockets[socket.id];
		delete  userindex[socket.id];
		delete lon1[socket.id];
		delete lon2[socket.id];
		delete lat1[socket.id];
		delete lat2[socket.id];
//		})();
	});
	socket.on('viewchange', function(data){
//		(function(){
		console.log('websocket user '+userindex[socket.id]+' viewchange: '+data.lon1+'-'+data.lon2+'/'+data.lat1+'-'+data.lat2);
		lon1[socket.id]=data.lon1;
		lon2[socket.id]=data.lon2;
		lat1[socket.id]=data.lat1;
		lat2[socket.id]=data.lat2;
//		})();
	});
});

http.listen(3000, function(){
	console.log('listening on *:3000');
});
