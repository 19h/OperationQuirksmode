

var common = require('../common');
var assert = require('assert');

var net = require('net');

var s = new net.Stream();

// test that destroy called on a stream with a server only ever decrements the
// server connection count once

s.server = new net.Server();
s.server.connections = 10;

assert.equal(10, s.server.connections);
s.destroy();
assert.equal(9, s.server.connections);
s.destroy();
assert.equal(9, s.server.connections);
