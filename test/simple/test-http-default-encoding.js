

var common = require('../common');
var assert = require('assert');
var http = require('http');

var expected = 'This is a unicode text: سلام';
var result = '';

var server = http.Server(function(req, res) {
  req.setEncoding('utf8');
  req.on('data', function(chunk) {
    result += chunk;
  }).on('end', function() {
    clearTimeout(timeout);
    server.close();
  });

  var timeout = setTimeout(function() {
    process.exit(1);
  }, 100);

  res.writeHead(200);
  res.end('hello world\n');
});

server.listen(common.PORT, function() {
  http.request({
    port: common.PORT,
    path: '/',
    method: 'POST'
  }, function(res) {
    console.log(res.statusCode);
    res.resume();
  }).on('error', function(e) {
    console.log(e.message);
    process.exit(1);
  }).end(expected);
});

process.on('exit', function() {
  assert.equal(expected, result);
});
