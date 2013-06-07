

var common = require('../common');
var assert = require('assert');
var Stream = require('stream').Stream;

(function testErrorListenerCatches() {
  var source = new Stream();
  var dest = new Stream();

  source.pipe(dest);

  var gotErr = null;
  source.on('error', function(err) {
    gotErr = err;
  });

  var err = new Error('This stream turned into bacon.');
  source.emit('error', err);
  assert.strictEqual(gotErr, err);
})();

(function testErrorWithoutListenerThrows() {
  var source = new Stream();
  var dest = new Stream();

  source.pipe(dest);

  var err = new Error('This stream turned into bacon.');

  var gotErr = null;
  try {
    source.emit('error', err);
  } catch (e) {
    gotErr = e;
  }

  assert.strictEqual(gotErr, err);
})();
