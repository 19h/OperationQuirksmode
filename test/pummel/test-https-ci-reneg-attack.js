

var common = require('../common');
var assert = require('assert');
var spawn = require('child_process').spawn;
var tls = require('tls');
var https = require('https');
var fs = require('fs');

// renegotiation limits to test
var LIMITS = [0, 1, 2, 3, 5, 10, 16];

if (process.platform === 'win32') {
  console.log('Skipping test, you probably don\'t have openssl installed.');
  process.exit();
}

(function() {
  var n = 0;
  function next() {
    if (n >= LIMITS.length) return;
    tls.CLIENT_RENEG_LIMIT = LIMITS[n++];
    test(next);
  }
  next();
})();

function test(next) {
  var options = {
    cert: fs.readFileSync(common.fixturesDir + '/test_cert.pem'),
    key: fs.readFileSync(common.fixturesDir + '/test_key.pem')
  };

  var seenError = false;

  var server = https.createServer(options, function(req, res) {
    var conn = req.connection;
    conn.on('error', function(err) {
      console.error('Caught exception: ' + err);
      assert(/TLS session renegotiation attack/.test(err));
      conn.destroy();
      seenError = true;
    });
    res.end('ok');
  });

  server.listen(common.PORT, function() {
    var args = ('s_client -connect 127.0.0.1:' + common.PORT).split(' ');
    var child = spawn('openssl', args);

    child.stdout.pipe(process.stdout);
    child.stderr.pipe(process.stderr);

    // count handshakes, start the attack after the initial handshake is done
    var handshakes = 0;
    var renegs = 0;

    child.stderr.on('data', function(data) {
      if (seenError) return;
      handshakes += (('' + data).match(/verify return:1/g) || []).length;
      if (handshakes === 2) spam();
      renegs += (('' + data).match(/RENEGOTIATING/g) || []).length;
    });

    child.on('exit', function() {
      assert.equal(renegs, tls.CLIENT_RENEG_LIMIT + 1);
      server.close();
      process.nextTick(next);
    });

    var closed = false;
    child.stdin.on('error', function(err) {
      assert.equal(err.code, 'ECONNRESET');
      closed = true;
    });
    child.stdin.on('close', function() {
      closed = true;
    });

    // simulate renegotiation attack
    function spam() {
      if (closed) return;
      child.stdin.write('R\n');
      setTimeout(spam, 50);
    }
  });
}
