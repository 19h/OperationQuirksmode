




var common = require('../common');
var assert = require('assert');

console.error('before');

var Script = process.binding('evals').NodeScript;

// undefined reference
var script = new Script('foo.bar = 5;');
script.runInNewContext();

console.error('after');
