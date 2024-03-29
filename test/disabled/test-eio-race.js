


var common = require('../common');
var assert = require('assert');

var count = 100;
var fs = require('fs');

// person.jpg is 57kb. We just need some file that is sufficently large.
var filename = require('path').join(common.fixturesDir, 'person.jpg');

function tryToKillEventLoop() {
  console.log('trying to kill event loop ...');

  fs.stat(__filename, function(err) {
    if (err) {
      throw new Exception('first fs.stat failed');
    } else {
      fs.stat(__filename, function(err) {
        if (err) {
          throw new Exception('second fs.stat failed');
        } else {
          console.log('could not kill event loop, retrying...');

          setTimeout(function() {
            if (--count) {
              tryToKillEventLoop();
            } else {
              console.log('done trying to kill event loop');
              process.exit(0);
            }
          }, 1);
        }
      });
    }
  });
}

// Generate a lot of thread pool events
var pos = 0;
fs.open(filename, 'r', 0666, function(err, fd) {
  if (err) throw err;

  function readChunk() {
    fs.read(fd, 1024, 0, 'binary', function(err, chunk, bytesRead) {
      if (err) throw err;
      if (chunk) {
        pos += bytesRead;
        //console.log(pos);
        readChunk();
      } else {
        fs.closeSync(fd);
        throw new Exception("Shouldn't get EOF");
      }
    });
  }
  readChunk();
});

tryToKillEventLoop();

process.on('exit', function() {
  console.log('done with test');
  assert.ok(pos > 10000);
});
