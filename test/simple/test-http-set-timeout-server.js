

var common = require('../common.js');
var assert = require('assert');
var http = require('http');
var net = require('net');

var tests = [];

function test(fn) {
  if (!tests.length)
    process.nextTick(run);
  tests.push(fn);
}

function run() {
  var fn = tests.shift();
  if (fn) {
    console.log('# %s', fn.name);
    fn(run);
  } else
    console.log('ok');
}

test(function serverTimeout(cb) {
  var caughtTimeout = false;
  process.on('exit', function() {
    assert(caughtTimeout);
  });
  var server = http.createServer(function(req, res) {
    // just do nothing, we should get a timeout event.
  });
  server.listen(common.PORT);
  server.setTimeout(50, function(socket) {
    caughtTimeout = true;
    socket.destroy();
    server.close();
    cb();
  });
  http.get({ port: common.PORT }).on('error', function() {});
});

test(function serverRequestTimeout(cb) {
  var caughtTimeout = false;
  process.on('exit', function() {
    assert(caughtTimeout);
  });
  var server = http.createServer(function(req, res) {
    // just do nothing, we should get a timeout event.
    req.setTimeout(50, function() {
      caughtTimeout = true;
      req.socket.destroy();
      server.close();
      cb();
    });
  });
  server.listen(common.PORT);
  var req = http.request({ port: common.PORT, method: 'POST' });
  req.on('error', function() {});
  req.write('Hello');
  // req is in progress
});

test(function serverResponseTimeout(cb) {
  var caughtTimeout = false;
  process.on('exit', function() {
    assert(caughtTimeout);
  });
  var server = http.createServer(function(req, res) {
    // just do nothing, we should get a timeout event.
    res.setTimeout(50, function() {
      caughtTimeout = true;
      res.socket.destroy();
      server.close();
      cb();
    });
  });
  server.listen(common.PORT);
  http.get({ port: common.PORT }).on('error', function() {});
});

test(function serverRequestNotTimeoutAfterEnd(cb) {
  var caughtTimeoutOnRequest = false;
  var caughtTimeoutOnResponse = false;
  process.on('exit', function() {
    assert(!caughtTimeoutOnRequest);
    assert(caughtTimeoutOnResponse);
  });
  var server = http.createServer(function(req, res) {
    // just do nothing, we should get a timeout event.
    req.setTimeout(50, function(socket) {
      caughtTimeoutOnRequest = true;
    });
    res.on('timeout', function(socket) {
      caughtTimeoutOnResponse = true;
    });
  });
  server.on('timeout', function(socket) {
    socket.destroy();
    server.close();
    cb();
  });
  server.listen(common.PORT);
  http.get({ port: common.PORT }).on('error', function() {});
});

test(function serverResponseTimeoutWithPipeline(cb) {
  var caughtTimeout = '';
  process.on('exit', function() {
    assert.equal(caughtTimeout, '/2');
  });
  var server = http.createServer(function(req, res) {
    res.setTimeout(50, function() {
      caughtTimeout += req.url;
    });
    if (req.url === '/1') res.end();
  });
  server.on('timeout', function(socket) {
    socket.destroy();
    server.close();
    cb();
  });
  server.listen(common.PORT);
  var c = net.connect({ port: common.PORT, allowHalfOpen: true }, function() {
    c.write('GET /1 HTTP/1.1\r\nHost: localhost\r\n\r\n');
    c.write('GET /2 HTTP/1.1\r\nHost: localhost\r\n\r\n');
    c.write('GET /3 HTTP/1.1\r\nHost: localhost\r\n\r\n');
  });
});

test(function idleTimeout(cb) {
  var caughtTimeoutOnRequest = false;
  var caughtTimeoutOnResponse = false;
  var caughtTimeoutOnServer = false;
  process.on('exit', function() {
    assert(!caughtTimeoutOnRequest);
    assert(!caughtTimeoutOnResponse);
    assert(caughtTimeoutOnServer);
  });
  var server = http.createServer(function(req, res) {
    req.on('timeout', function(socket) {
      caughtTimeoutOnRequest = true;
    });
    res.on('timeout', function(socket) {
      caughtTimeoutOnResponse = true;
    });
    res.end();
  });
  server.setTimeout(50, function(socket) {
    caughtTimeoutOnServer = true;
    socket.destroy();
    server.close();
    cb();
  });
  server.listen(common.PORT);
  var c = net.connect({ port: common.PORT, allowHalfOpen: true }, function() {
    c.write('GET /1 HTTP/1.1\r\nHost: localhost\r\n\r\n');
    // Keep-Alive
  });
});
