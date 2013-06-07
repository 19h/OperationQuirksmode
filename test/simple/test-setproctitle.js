

// Original test written by Jakub Lekstan <kuebzky@gmail.com>

// FIXME add sunos support
if ('linux freebsd darwin'.indexOf(process.platform) === -1) {
  console.error('Skipping test, platform not supported.');
  process.exit();
}

var common = require('../common');
var assert = require('assert');
var exec = require('child_process').exec;

var title = 'testTestTESTtest123123123123123123HiHaiJo';

assert.notEqual(process.title, title);
process.title = title;
assert.equal(process.title, title);

exec('ps -p ' + process.pid + ' -o args=', function(error, stdout, stderr) {
  assert.equal(error, null);
  assert.equal(stderr, '');

  // freebsd always add ' (procname)' to the process title
  if (process.platform === 'freebsd') title += ' (node)';

  // omitting trailing whitespace and \n
  assert.equal(stdout.replace(/\s+$/, ''), title);
});
