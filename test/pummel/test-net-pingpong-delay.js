

var common = require('../common');
var assert = require('assert');
var net = require('net');


var tests_run = 0;

function pingPongTest(port, host, on_complete) {
  var N = 100;
  var DELAY = 1;
  var count = 0;
  var client_ended = false;

  var server = net.createServer({ allowHalfOpen: true }, function(socket) {
    socket.setEncoding('utf8');

    socket.on('data', function(data) {
      console.log(data);
      assert.equal('PING', data);
      assert.equal('open', socket.readyState);
      assert.equal(true, count <= N);
      setTimeout(function() {
        assert.equal('open', socket.readyState);
        socket.write('PONG');
      }, DELAY);
    });

    socket.on('timeout', function() {
      common.debug('server-side timeout!!');
      assert.equal(false, true);
    });

    socket.on('end', function() {
      console.log('server-side socket EOF');
      assert.equal('writeOnly', socket.readyState);
      socket.end();
    });

    socket.on('close', function(had_error) {
      console.log('server-side socket.end');
      assert.equal(false, had_error);
      assert.equal('closed', socket.readyState);
      socket.server.close();
    });
  });

  server.listen(port, host, function() {
    var client = net.createConnection(port, host);

    client.setEncoding('utf8');

    client.on('connect', function() {
      assert.equal('open', client.readyState);
      client.write('PING');
    });

    client.on('data', function(data) {
      console.log(data);
      assert.equal('PONG', data);
      assert.equal('open', client.readyState);

      setTimeout(function() {
        assert.equal('open', client.readyState);
        if (count++ < N) {
          client.write('PING');
        } else {
          console.log('closing client');
          client.end();
          client_ended = true;
        }
      }, DELAY);
    });

    client.on('timeout', function() {
      common.debug('client-side timeout!!');
      assert.equal(false, true);
    });

    client.on('close', function() {
      console.log('client.end');
      assert.equal(N + 1, count);
      assert.ok(client_ended);
      if (on_complete) on_complete();
      tests_run += 1;
    });
  });
}

pingPongTest(common.PORT);

process.on('exit', function() {
  assert.equal(1, tests_run);
});
