

var common = require('../common');
var assert = require('assert');
var stream = require('stream');
var util = require('util');

try {
  var crypto = require('crypto');
} catch (e) {
  console.log('Not compiled with OPENSSL support.');
  process.exit();
}

// Small stream to buffer converter
function Stream2buffer(callback) {
  stream.Writable.call(this);

  this._buffers = [];
  this.once('finish', function () {
    callback(null, Buffer.concat(this._buffers));
  });
}
util.inherits(Stream2buffer, stream.Writable);

Stream2buffer.prototype._write = function (data, encodeing, done) {
  this._buffers.push(data);
  return done(null);
};

// Create an md5 hash of "Hallo world"
var hasher1 = crypto.createHash('md5');
    hasher1.pipe(new Stream2buffer(common.mustCall(function end(err, hash) {
      assert.equal(err, null);
      assert.equal(hash.toString('hex'), '06460dadb35d3d503047ce750ceb2d07');
    })));
    hasher1.end('Hallo world');

// Simpler check for unpipe, setEncoding, pause and resume
crypto.createHash('md5').unpipe({});
crypto.createHash('md5').setEncoding('utf8');
crypto.createHash('md5').pause();
crypto.createHash('md5').resume();
