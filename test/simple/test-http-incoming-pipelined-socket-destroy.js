

var common = require('../common');
var assert = require('assert');

var http = require('http');
var net = require('net');


// Set up some timing issues where sockets can be destroyed
// via either the req or res.
var server = http.createServer(function(req, res) {
  switch (req.url) {
    case '/1':
      return setTimeout(function() {
        req.socket.destroy();
      });

    case '/2':
      return process.nextTick(function() {
        res.destroy();
      });

    // in one case, actually send a response in 2 chunks
    case '/3':
      res.write('hello ');
      return setTimeout(function() {
        res.end('world!');
      });

    default:
      return res.destroy();
  }
});


// Make a bunch of requests pipelined on the same socket
function generator() {
  var reqs = [ 3, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 ];
  return reqs.map(function(r) {
    return 'GET /' + r + ' HTTP/1.1\r\n' +
           'Host: localhost:' + common.PORT + '\r\n' +
           '\r\n' +
           '\r\n'
  }).join('');
}


server.listen(common.PORT, function() {
  var client = net.connect({ port: common.PORT });

  // immediately write the pipelined requests.
  // Some of these will not have a socket to destroy!
  client.write(generator(), function() {
    server.close();
  });
});

process.on('exit', function(c) {
  if (!c)
    console.log('ok');
});
