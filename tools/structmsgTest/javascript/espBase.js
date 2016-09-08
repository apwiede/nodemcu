/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * core functions for Esp (espBase.js)
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

/* make an Array behave like other Esp objects when debugging ! */

/* =============================== Array toDebugString ============ */

Array.prototype.toDebugString = function() {
    var arr = this;
    return "Array!"+arr.length+"!"+arr.toString();
}

/* =============================== Array mySelf ============ */

Array.prototype.mySelf = function() {
    var arr = this;
    return "Array!";
}

/* =============================== Esp Global Variable ====================== */

if (typeof Esp != 'undefined') {
    Esp._Esp = Esp;
}


/* =============================== Esp Global Object ====================== */

var Esp = function() {
    var EM = this;
    var instanceOf = function(o, type) {
        return (o && o.hasOwnProperty && (o instanceof type));
    };

    if (!(instanceOf(EM, Esp))) {
        EM = new Esp();
    } else {
        EM._init();
    }
    EM.instanceOf = instanceOf;
    EM.my_name = "Esp";
    return EM;
};


/* =============================== Esp Global Initialization ====================== */

(function() {
    var proto;
    var VERSION = '0.0.1';
    var BASE = 'Esp';
    var NOOP = function() {};
    var time = new Date().getTime();

    /* ======================= proto ================================== */

    proto = {

        /* ======================= proto logPrint ================================== */

        logPrint: function(msg, cat, src) {
            if (typeof window != 'undefined' && typeof window.console != 'undefined') {
                console.log("LOG-Esp:\t"+cat+"\t"+src+"\t"+msg+"!");
            } else {
                if (typeof alert != 'undefined') {
                    alert("No console for logging: "+"LOG-Esp:\t"+cat+"\t"+src+"\t"+msg+"!");
                } else {
                    print("No console for logging: "+"LOG-Esp:\t"+cat+"\t"+src+"\t"+msg+"!");
                }
            }
        },

        /* ======================= proto _init ================================== */

        /**
         * Initialize this EM instance
         * @private
         * @method _init
         */
        _init: function() {
            var EM = this;
            var Env = EM.Env;

            EM.version = VERSION;
            if (!Env) {
                EM.Env = {
                    mods:     {}, // flat module map
                    versions: {}, // version module map
                    base:     BASE,
                };

                // configuration defaults
                EM.config = EM.config || {
                    debug:             true,
                    useBrowserConsole: true,
                    logFn:             EM.logPrint
                };
            }
        },

        /* ======================= proto addModule ========================== */

        addModule: function(name, fn, version, details) {
            details = details || {};
            var env = EM.Env;
            var mod = {
                name:    name,
                fn:      fn,
                version: version,
                details: details
            };
            env.mods[name] = mod;
        },

        /* ======================= proto log function ========================== */

        // this is replaced if the log module is included
        log: NOOP,

        /* ======================= proto message function ====================== */

        message: NOOP,

        /* ======================= proto dump function ========================== */

        // this is replaced if the dump module is included
        dump: function (o) {
            return ''+o; 
        },

        /* ======================= proto error function ========================= */

        /**
         * Report an error.
         * @method error
         * @param msg {String} the error message.
         * @param e {Error|String} Optional JS error that was caught, or an error string.
         * @param data Optional additional info
         * @return {EM} this EM instance.
         */
        error: function(msg, e, data) {
            var EM = this;

            EM.logPrint(msg, e, data);
            return EM;
        }

    };

    Esp.prototype = proto;
    //
    // inheritance utilities are not available yet
    for (prop in proto) {
        if (proto.hasOwnProperty(prop)) {
            Esp[prop] = proto[prop];
        }
    }

    // set up the environment
    Esp._init();

}());

/***********************************************************************************
 * This is the STARTING POINT !!
 *
 * Variable EM is the instance variable of an Esp Instance.
 */

var EM = Esp();

// next lines for testing only!!
EM.logPrint("start", "info", "espMsg");
