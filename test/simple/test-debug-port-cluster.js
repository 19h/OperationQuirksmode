

var common = require('../common');
var assert = require('assert');
var spawn = require('child_process').spawn;

var port = common.PORT + 1337;

var args = [
  '--debug=' + port,
  common.fixturesDir + '/clustered-server/app.js'
];

var child = spawn(process.execPath, args);
var outputLines = [];

child.stderr.on('data', function(data) {
  var lines = data.toString().replace(/\r/g, '').trim().split('\n');
  var line = lines[0];

  lines.forEach(function(ln) { console.log('> ' + ln) } );

  if (line === 'all workers are running') {
    assertOutputLines();
    process.exit();
  } else {
    outputLines = outputLines.concat(lines);
  }
});

setTimeout(function testTimedOut() {
  assert(false, 'test timed out.');
}, 3000);

process.on('exit', function() {
    child.kill();
});

function assertOutputLines() {
  var expectedLines = [
    'debugger listening on port ' + port,
    'debugger listening on port ' + (port+1),
    'debugger listening on port ' + (port+2),
  ];

  // Do not assume any particular order of output messages,
  // since workers can take different amout of time to
  // start up
  outputLines.sort();

  assert.equal(outputLines.length, expectedLines.length)
  for (var i = 0; i < expectedLines.length; i++)
    assert.equal(outputLines[i], expectedLines[i]);
}
