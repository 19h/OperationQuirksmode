

// Ensure that if a dgram socket is closed before the DNS lookup completes, it
// won't crash.

var assert = require('assert'),
    common = require('../common'),
    dgram = require('dgram');

var buf = new Buffer(1024);
buf.fill(42);

var socket = dgram.createSocket('udp4');

socket.send(buf, 0, buf.length, common.port, 'localhost');
socket.close();
