// UDP 通信部分

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
//			console.log("sending to user "+userindex[uidx]);
			usersockets[uidx].emit('ADSB', obj);
		} 
//		else console.log("do not sending to user "+userindex[uidx]);
	}
});


// WEB界面部分
// vendor libraries
var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var session = require('express-session');
var bcrypt = require('bcrypt-nodejs');
var ejs = require('ejs');
var path = require('path');
var passport = require('passport');
var LocalStrategy = require('passport-local').Strategy;

// custom libraries
// routes
var route = require('./route');
// model
var Model = require('./model');

var app = express();

passport.use(new LocalStrategy(function(username, password, done) {
   new Model.User({username: username}).fetch().then(function(data) {
      var user = data;
      if(user === null) {
         return done(null, false, {message: 'Invalid username or password'});
      } else {
         user = data.toJSON();
         if(!bcrypt.compareSync(password, user.password)) {
            return done(null, false, {message: 'Invalid username or password'});
         } else {
            return done(null, user);
         }
      }
   });
}));

passport.serializeUser(function(user, done) {
  done(null, user.username);
});

passport.deserializeUser(function(username, done) {
   new Model.User({username: username}).fetch().then(function(user) {
      done(null, user);
   });
});

app.set('port', process.env.PORT || 3001);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');

app.use(cookieParser());
app.use(bodyParser());
app.use(session({secret: 'secret strategic xxzzz code'}));
app.use(passport.initialize());
app.use(passport.session());

app.use(express.static('html'));

// GET
app.get('/', route.index);

// signin
// GET
app.get('/signin', route.signIn);
// POST
app.post('/signin', route.signInPost);

// signup
// GET
//app.get('/signup', route.signUp);
// POST
//app.post('/signup', route.signUpPost);

// logout
// GET
app.get('/signout', route.signOut);

/********************************/

/********************************/
// 404 not found
app.use(route.notFound404);

/*
var server = app.listen(app.get('port'), function(err) {
   if(err) throw err;

   var message = 'Server is running @ http://localhost:' + server.address().port;
   console.log(message);
});

*/


var http = require('http').Server(app);
var io = require('socket.io')(http);


/*
app.get('/', function(req, res){
   if(!req.isAuthenticated()) {
      res.redirect('/signin');
   } else {

      var user = req.user;

      if(user !== undefined) {
         user = user.toJSON();
      }
      //res.render('index', {title: 'Home', user: user});
	console.log('user '+user + ' access /');
	res.sendFile(__dirname + 'html' + '/index.html');
   }

});
*/


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
//		console.log('websocket user '+userindex[socket.id]+' viewchange: '+data.lon1+'-'+data.lon2+'/'+data.lat1+'-'+data.lat2);
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
