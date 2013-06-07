

var common = require('../common');
var tls = require('tls');
var net = require('net');
var fs = require('fs');
var assert = require('assert');

var options = {
  key: fs.readFileSync(common.fixturesDir + '/test_key.pem'),
  cert: fs.readFileSync(common.fixturesDir + '/test_cert.pem')
};

var server = tls.createServer(options, function(c) {

}).listen(common.PORT, function() {
  var client = net.connect(common.PORT, function() {
    var bonkers = new Buffer(1024 * 1024);
    bonkers.fill(42);
    client.end(bonkers);
  });

  var once = false;
  client.on('error', function() {
    if (!once) {
      once = true;
      client.destroy();
      server.close();
    }
  });
});
