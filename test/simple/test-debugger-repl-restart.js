

var repl = require('./helper-debugger-repl.js');

repl.startDebugger('breakpoints.js');

// Restart the debugged script
repl.addTest('restart', [
  /terminated/,
].concat(repl.initialLines));

repl.addTest('quit', []);
