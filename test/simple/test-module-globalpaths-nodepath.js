

var common = require('../common');
var assert = require('assert');

var module = require('module');

var isWindows = process.platform === 'win32';

var partA, partB;

if (isWindows) {
  partA = 'C:\\Users\\Rocko Artischocko\\AppData\\Roaming\\npm';
  partB = 'C:\\Program Files (x86)\\nodejs\\';
  process.env['NODE_PATH'] = partA + ';' + partB;
} else {
  partA = '/usr/test/lib/node_modules';
  partB = '/usr/test/lib/node';
  process.env['NODE_PATH'] = partA + ':' + partB;
}

module._initPaths();

assert.ok(module.globalPaths.indexOf(partA) !== -1);
assert.ok(module.globalPaths.indexOf(partB) !== -1);

assert.ok(Array.isArray(module.globalPaths));