/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * log functions for Esp (espLog.js)
 *
 * Part of this code is taken from:
 * http://yuilibrary.com/ YUI 3.3 version
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 *
 */

/* this is the log module with logging functionality */

EM.addModule("Esp-log", function(T, name) {
    var INSTANCE = T;
    var LOGEVENT = 'esp:log';
    var UNDEFINED = 'undefined';
    var LEVELS = {
        debug: 1,
        info:  1,
        warn:  1,
        error: 1,
        log:   1
    };
    var START_LEVELS = {
        debug: 1,
        info:  1,
        warn:  1,
        error: 1,
        log:   1
    };
    var log_switches = {
        useBrowserConsole: true,
        debug:             false,
        logExclude:        null,
        logInclude:        null,
        logFn:             null,
    };


    /* =============================== INSTANCE.log ================================== */

    /**
     * If the 'debug' config is true, a 'at:log' event will be
     * dispatched, which the Console widget and anything else
     * can consume.  If the 'useBrowserConsole' config is true, it will
     * write to the browser console if available.  EM-specific log
     * messages will only be present in the -debug versions of the
     * JS files.  The build system is supposed to remove log statements
     * from the raw and minified versions of the files.
     *
     * @method log
     * @for EM
     * @param  {String}  msg  The message to log.
     * @param  {String}  cat  The log category for the message.  Default
     *                        categories are "info", "warn", "error", time".
     *                        Custom categories can be used as well. (opt).
     * @param  {String}  src  The source of the the message (opt).
     * @param  {boolean} silent If true, the log event won't fire.
     * @return {EM}      EM instance.
     */

    INSTANCE.log = function(msg, cat, src, silent) {
        var bail = false;
        var excl;
        var incl;
        var m;
        var f;
        var T = INSTANCE;
        var c = log_switches;
        if (typeof T.config != "undefined") {
            c = T.config ;
        }
//        var publisher = (T.fire) ? T : EM.Env.globalEvents;
        var publisher = null;
        // suppress log message if the config is off or the event stack
        // or the event call stack contains a consumer of the tui:log event
        if (c.debug) {
            var cat_parts = cat.split(".");
            var cat_level = -1;
            if (cat_parts.length > 1) {
                cat_level = parseInt(cat_parts[0], 10);
                cat = cat_parts[1];
            }
            var f = (cat && (typeof console != "undefined") && console.log && console[cat] && (cat in LEVELS)) ? cat : 'log';
            var show_it = false;
            var start_level = 99;
            if (f in START_LEVELS) {
                start_level = START_LEVELS[f];
                var my_level = LEVELS[f];
                if (start_level >= cat_level) {
                    show_it = true;
                }
            } else {
                T.logPrint("funny F!"+f+"!", "ERROR", "Esp-log");
            }
            // apply source filters
            if (show_it && src) {
                excl = c.logExclude;
                incl = c.logInclude;
                if (incl && !(src in incl)) {
                    bail = 1;
                } else {
                    if (incl && (src in incl)) {
                        bail = !incl[src];
                    } else {
                        if (excl && (src in excl)) {
                          bail = excl[src];
                        }
                    }
                }
                if (!bail) {
                    if (c.useBrowserConsole) {
                        m = (src) ? src + ': ' + msg : msg;
                        if (T.Util && T.Util.isFunction(c.logFn)) {
                            c.logFn.call(T, msg, cat, src);
                        } else {
                            if (typeof console != UNDEFINED && console.log) {
	                        if (show_it) {
                                    console[f](m);
	                        }
                            } else{
                                if (typeof opera != UNDEFINED) {
                                    opera.postError(m);
                                } else {
                                    f = (cat && (cat in LEVELS)) ? cat : 'log';
	                            var show_it = false;
	                            var start_level = 99;
	                            if (f in START_LEVELS) {
                                        start_level = START_LEVELS[f];
                                        var my_level = LEVELS[f];
                                        if (start_level >= cat_level) {
	                                    show_it = true;
		                        }
	                            } else {
                                        T.logPrint("funny F 2!"+f+"!", "error", "Esp-log");
	                            }
                                    if (show_it) {
                                        T.logPrint(m, cat, "Esp-log");
                                        print(cat+": "+m);
		                    }
	                        }
                            }
                        }
                    }
                    if (publisher && !silent) {
                        if (publisher == T && (!publisher.getEvent(LOGEVENT))) {
                            publisher.publish(LOGEVENT, {
                                broadcast: 2
                           });
                        }
                        publisher.fire(LOGEVENT, {
                            msg: msg,
                            cat: cat,
                            src: src
                        });
                    }
                }
            }
        }
        return T;
    };

    /* =============================== INSTANCE.message ================================== */

    /**
     * Write a system message.  This message will be preserved in the
     * minified and raw versions of the EM files, unlike log statements.
     * @method message
     * @for EM
     * @param  {String}  msg  The message to log.
     * @param  {String}  cat  The log category for the message.  Default
     *                        categories are "info", "warn", "error", time".
     *                        Custom categories can be used as well. (opt).
     * @param  {String}  src  The source of the the message (opt).
     * @param  {boolean} silent If true, the log event won't fire.
     * @return {EM}      EM instance.
     */

    INSTANCE.message = function() {
        return INSTANCE.log.apply(INSTANCE, arguments);
    };

    INSTANCE.log("module: "+name+" initialised!", "info", "log.js");

}, "0.0.1", {});

