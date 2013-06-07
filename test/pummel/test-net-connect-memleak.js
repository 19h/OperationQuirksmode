

// Flags: --expose-gc

var common = require('../common');
var assert = require('assert');
var net = require('net');

assert(typeof gc === 'function', 'Run this test with --expose-gc');
net.createServer(function() {}).listen(common.PORT);

(function() {
  // 2**26 == 64M entries
  for (var i = 0, junk = [0]; i < 26; ++i) junk = junk.concat(junk);

  net.createConnection(common.PORT, '127.0.0.1', function() {
    assert(junk.length != 0);  // keep reference alive
    setTimeout(done, 10);
    gc();
  });
})();

function done() {
  var before = process.memoryUsage().rss;
  gc();
  var after = process.memoryUsage().rss;
  var reclaimed = (before - after) / 1024;
  console.log('%d kB reclaimed', reclaimed);
  assert(reclaimed > 256 * 1024);  // it's more like 512M on x64
  process.exit();
}
