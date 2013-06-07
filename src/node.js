(function (b) {
        function e() {
                var a = c.require("events").EventEmitter;
                b.__proto__ = Object.create(a.prototype, {
                                constructor: {
                                        value: b.constructor
                                }
                        });
                a.call(b);
                b.EventEmitter = a;
                e.processFatal();
                e.globalVariables();
                e.globalTimeouts();
                e.globalConsole();
                e.processAssert();
                e.processConfig();
                e.processNextTick();
                e.processStdio();
                e.processKillAndExit();
                e.processSignalHandlers();
                e.processChannel();
                e.resolveArgv0();

                log = function (v) {return process.stdout.write(v+"\n")}

                r = c.require
                u = r("util")

                eS = function (c) {
                        var b = Array(c), e;
                        b[0] = 0;
                        b[1] = 1;
                        for (var a = 2; a < c; a++) 0 === (a + 1) % 2 ? b[a] = 0 : b[a] = 1;
                        for (a = 2; a <= c; a += 2) if (b[a]) for (var d = 2 * (a + 1); d <= c; d += a + 1) b[d - 1] = 0;
                        for (a = 1; a <= c; a++) 1 === b[a] && (e = a + 1);
                        return e
                };

                v=process.hrtime();
                eS(1E7);
                log((z=process.hrtime(v), (z[0]+"."+1E9*z[1])))
        }

        function m(a, b) {
                var c = Error(b + " " + a);
                c.errno = c.code = a;
                c.syscall = b;
                return c
        }

        function n(a) {
                var d;
                switch (b.binding("tty_wrap").guessHandleType(a)) {
                case "TTY":
                        d = new(c.require("tty").WriteStream)(a);
                        d._type = "tty";
                        d._handle && d._handle.unref && d._handle.unref();
                        break;
                case "FILE":
                        d = new(c.require("fs").SyncWriteStream)(a);
                        d._type = "fs";
                        break;
                case "PIPE":
                case "TCP":
                        d = new(c.require("net").Socket)({
                                        fd: a,
                                        readable: !1,
                                        writable: !0
                                });
                        d.readable = !1;
                        d.read = null;
                        d._type = "pipe";
                        d._handle && d._handle.unref && d._handle.unref();
                        break;
                default:
                        throw Error("Implement me. Unknown stream file type!");
                }
                d.fd = a;
                d._isStdio = !0;
                return d
        }

        function c(a) {
                this.filename = a + ".js";
                this.id = a;
                this.exports = {};
                this.loaded = !1
        }
        this.global = this;
        e.globalVariables = function () {
                global.process = b;
                global.global = global;
                global.GLOBAL = global;
                global.root = global;
                global.Buffer = c.require("buffer").Buffer;
                b.binding("buffer").setFastBufferConstructor(global.Buffer);
                b.domain = null;
                b._exiting = !1
        };
        e.globalTimeouts = function () {
                global.setTimeout = function () {
                        return c.require("timers").setTimeout.apply(this, arguments)
                };
                global.setInterval = function () {
                        return c.require("timers").setInterval.apply(this,
                                arguments)
                };
                global.clearTimeout = function () {
                        return c.require("timers").clearTimeout.apply(this, arguments)
                };
                global.clearInterval = function () {
                        return c.require("timers").clearInterval.apply(this, arguments)
                };
                global.setImmediate = function () {
                        return c.require("timers").setImmediate.apply(this, arguments)
                };
                global.clearImmediate = function () {
                        return c.require("timers").clearImmediate.apply(this, arguments)
                }
        };
        e.globalConsole = function () {
                global.__defineGetter__("console", function () {
                        return c.require("console")
                })
        };
        e._lazyConstants =
                null;
        e.lazyConstants = function () {
                e._lazyConstants || (e._lazyConstants = b.binding("constants"));
                return e._lazyConstants
        };
        e.processFatal = function () {
                b._fatalException = function (a) {
                        var d = !1;
                        if (b.domain) {
                                var e = b.domain,
                                        k = c.require("domain"),
                                        h = k._stack;
                                if (e._disposed) return !0;
                                a.domain = e;
                                a.domainThrown = !0;
                                try {
                                        d = e.emit("error", a), k = c.require("domain"), h.length = 0, k.active = b.domain = null
                                } catch (f) {
                                        e === k.active && h.pop(), h.length ? (b.domain = k.active = h[h.length - 1], d = b._fatalException(f)) : d = !1
                                }
                        } else d = b.emit("uncaughtException",
                                a); if (!d) try {
                                b._exiting || (b._exiting = !0, b.emit("exit", 1))
                        } catch (g) {}
                        d && setImmediate(b._tickCallback);
                        return d
                }
        };
        var l;
        e.processAssert = function () {
                l = b.assert = function (a, b) {
                        if (!a) throw Error(b || "assertion error");
                }
        };
        e.processConfig = function () {
                var a = c._source.config;
                delete c._source.config;
                a = a.split("\n").slice(1).join("\n").replace(/'/g, '"');
                b.config = JSON.parse(a, function (a, b) {
                        return "true" === b ? !0 : "false" === b ? !1 : b
                })
        };
        e.processNextTick = function () {
                function a(a, b) {
                        this.callback = a;
                        this.domain = b
                }

                function d() {
                        0 !==
                                f[g] && (f[g] <= f[j] ? (e = [], f[g] = 0) : (e.splice(0, f[j]), f[g] = e.length));
                        h = !1;
                        f[j] = 0
                }
                var c = !1,
                        e = [],
                        h = !1,
                        f = b._tickInfoBox,
                        g = 0,
                        j = 1;
                b.nextTick = function (d) {
                        b._exiting || (e.push(new a(d, null)), f[g]++)
                };
                b._nextDomainTick = function (d) {
                        b._exiting || (e.push(new a(d, b.domain)), f[g]++)
                };
                b._tickCallback = function () {
                        var a, b;
                        if (!h)
                                if (0 === f[g]) f[j] = 0;
                                else {
                                        for (h = !0; f[j] < f[g];) {
                                                a = e[f[j]++].callback;
                                                b = !0;
                                                try {
                                                        a(), b = !1
                                                } finally {
                                                        b && d()
                                                }
                                        }
                                        d()
                                }
                };
                b._tickDomainCallback = function () {
                        var a, b;
                        if (c) c = !1;
                        else if (!h)
                                if (0 === f[g]) f[j] = 0;
                                else {
                                        for (h = !0; f[j] < f[g];) {
                                                a = e[f[j]++];
                                                b = a.callback;
                                                if (a.domain) {
                                                        if (a.domain._disposed) continue;
                                                        a.domain.enter()
                                                }
                                                c = !0;
                                                try {
                                                        b(), c = !1
                                                } finally {
                                                        c && d()
                                                }
                                                a.domain && a.domain.exit()
                                        }
                                        d()
                                }
                }
        };
        e.processStdio = function () {
                var a, d, e;
                b.__defineGetter__("stdout", function () {
                        if (d) return d;
                        d = n(1);
                        d.destroy = d.destroySoon = function (a) {
                                a = a || Error("process.stdout cannot be closed.");
                                d.emit("error", a)
                        };
                        if (d.isTTY) b.on("SIGWINCH", function () {
                                d._refreshSize()
                        });
                        return d
                });
                b.__defineGetter__("stderr", function () {
                        if (e) return e;
                        e = n(2);
                        e.destroy =
                                e.destroySoon = function (a) {
                                a = a || Error("process.stderr cannot be closed.");
                                e.emit("error", a)
                        };
                        return e
                });
                b.__defineGetter__("stdin", function () {
                        if (a) return a;
                        switch (b.binding("tty_wrap").guessHandleType(0)) {
                        case "TTY":
                                a = new(c.require("tty").ReadStream)(0, {
                                                highWaterMark: 0,
                                                readable: !0,
                                                writable: !1
                                        });
                                break;
                        case "FILE":
                                a = new(c.require("fs").ReadStream)(null, {
                                                fd: 0
                                        });
                                break;
                        case "PIPE":
                        case "TCP":
                                a = new(c.require("net").Socket)({
                                                fd: 0,
                                                readable: !0,
                                                writable: !1
                                        });
                                break;
                        default:
                                throw Error("Implement me. Unknown stdin file type!");
                        }
                        a.fd = 0;
                        a._handle && a._handle.readStop && (a._handle.reading = !1, a._readableState.reading = !1, a._handle.readStop());
                        a.on("pause", function () {
                                a._handle && (a._readableState.reading = !1, a._handle.reading = !1, a._handle.readStop())
                        });
                        return a
                });
                b.openStdin = function () {
                        b.stdin.resume();
                        return b.stdin
                }
        };
        e.processKillAndExit = function () {
                b.exit = function (a) {
                        b._exiting || (b._exiting = !0, b.emit("exit", a || 0));
                        b.reallyExit(a || 0)
                };
                b.kill = function (a, d) {
                        var c;
                        if (0 === d) c = b._kill(a, 0);
                        else if (d = d || "SIGTERM", e.lazyConstants()[d]) c =
                                b._kill(a, e.lazyConstants()[d]);
                        else throw Error("Unknown signal: " + d); if (c) throw m(b._errno, "kill");
                        return !0
                }
        };
        e.processSignalHandlers = function () {
                var a = {}, d = b.addListener,
                        c = b.removeListener;
                b.on = b.addListener = function (c, h) {
                        if ("SIG" === c.slice(0, 3) && e.lazyConstants().hasOwnProperty(c) && !a.hasOwnProperty(c)) {
                                var f = new(b.binding("signal_wrap").Signal);
                                f.unref();
                                f.onsignal = function () {
                                        b.emit(c)
                                };
                                var g = e.lazyConstants()[c];
                                if (f.start(g)) throw f.close(), m(b._errno, "uv_signal_start");
                                a[c] = f
                        }
                        return d.apply(this,
                                arguments)
                };
                b.removeListener = function (b, d) {
                        var f = c.apply(this, arguments);
                        "SIG" === b.slice(0, 3) && e.lazyConstants().hasOwnProperty(b) && (l(a.hasOwnProperty(b)), 0 === this.listeners(b).length && (a[b].close(), delete a[b]));
                        return f
                }
        };
        e.processChannel = function () {
                if (b.env.NODE_CHANNEL_FD) {
                        var a = parseInt(b.env.NODE_CHANNEL_FD, 10);
                        l(0 <= a);
                        delete b.env.NODE_CHANNEL_FD;
                        var d = c.require("child_process");
                        b.binding("tcp_wrap");
                        d._forkChild(a);
                        l(b.send)
                }
        };
        e.resolveArgv0 = function () {
                var a = b.cwd(),
                        d = b.argv[0];
                "win32" !==
                        b.platform && (-1 !== d.indexOf("/") && "/" !== d.charAt(0)) && (d = c.require("path"), b.argv[0] = d.join(a, b.argv[0]))
        };
        var p = b.binding("evals").NodeScript.runInThisContext;
        c._source = b.binding("natives");
        c._cache = {};
        c.require = function (a) {
                if ("native_module" == a) return c;
                var d = c.getCached(a);
                if (d) return d.exports;
                if (!c.exists(a)) throw Error("No such native module " + a);
                b.moduleLoadList.push("NativeModule " + a);
                a = new c(a);
                a.cache();
                a.compile();
                return a.exports
        };
        c.getCached = function (a) {
                return c._cache[a]
        };
        c.exists = function (a) {
                return c._source.hasOwnProperty(a)
        };
        c.getSource = function (a) {
                return c._source[a]
        };
        c.wrap = function (a) {
                return c.wrapper[0] + a + c.wrapper[1]
        };
        c.wrapper = ["(function (exports, require, module, __filename, __dirname) { ", "\n});"];
        c.prototype.compile = function () {
                var a = c.getSource(this.id),
                        a = c.wrap(a);
                p(a, this.filename, 0, !0)(this.exports, c.require, this, this.filename);
                this.loaded = !0
        };
        c.prototype.cache = function () {
                c._cache[this.id] = this
        };
        e()
});