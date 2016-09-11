/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg inheritUtil for Esp (inheritUtil.js)
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

EM.addModule("Esp-inheritUtil", function(T, name) {

    /* =============================== InheritUtil ================================== */

    var A            = T.Array;
    var OP           = Object.prototype;
    var CLONE_MARKER = '_~espm~_';
    var hasOwn   = OP.hasOwnProperty;
    var toString = OP.toString;

    /* =============================== dispatch ================================== */

    function dispatch(o, f, c, proto, action) {
        if (o && o[action] && o !== T) {
            return o[action].call(o, f, c);
        } else {
            switch (A.test(o)) {
            case 1:
                return A[action](o, f, c);
            case 2:
                return A[action](T.Array(o, 0, true), f, c);
            default:
                return T.Object[action](o, f, c, proto);
            }
        }
    }

    /* =============================== augment ================================== */

    /**
    Augments the _receiver_ with prototype properties from the _supplier_. The
    receiver may be a constructor function or an object. The supplier must be a
    constructor function.
    
    If the _receiver_ is an object, then the _supplier_ constructor will be called
    immediately after _receiver_ is augmented, with _receiver_ as the `this` object.
    
    If the _receiver_ is a constructor function, then all prototype methods of
    _supplier_ that are copied to _receiver_ will be sequestered, and the
    _supplier_ constructor will not be called immediately. The first time any
    sequestered method is called on the _receiver_'s prototype, all sequestered
    methods will be immediately copied to the _receiver_'s prototype, the
    _supplier_'s constructor will be executed, and finally the newly unsequestered
    method that was called will be executed.

    This sequestering logic sounds like a bunch of complicated voodoo, but it makes
    it cheap to perform frequent augmentation by ensuring that suppliers'
    constructors are only called if a supplied method is actually used. If none of
    the supplied methods is ever used, then there's no need to take the performance
    hit of calling the _supplier_'s constructor.

    @method augment
    @param {Function|Object} receiver Object or function to be augmented.
    @param {Function} supplier Function that supplies the prototype properties with
      which to augment the _receiver_.
    @param {Boolean} [overwrite=false] If `true`, properties already on the receiver
      will be overwritten if found on the supplier's prototype.
    @param {String[]} [whitelist] An array of property names. If specified,
      only the whitelisted prototype properties will be applied to the receiver, and
      all others will be ignored.
    @param {Array|any} [args] Argument or array of arguments to pass to the
      supplier's constructor when initializing.
    @return {Function} Augmented object.
    @for EM
    **/
    T.augment = function (receiver, supplier, overwrite, whitelist, args) {
        var rProto    = receiver.prototype;
        var sequester = rProto && supplier;
        var sProto    = supplier.prototype;
        var to        = rProto || receiver;
        var copy;
        var newPrototype;
        var replacements;
        var sequestered;
        var unsequester;

        args = args ? T.Array(args) : [];
        if (sequester) {
            newPrototype = {};
            replacements = {};
            sequestered  = {};

            copy = function (value, key) {
                if (overwrite || !(key in rProto)) {
                    if (toString.call(value) === '[object Function]') {
                        sequestered[key] = value;
                        newPrototype[key] = replacements[key] = function () {
                            return unsequester(this, value, arguments);
                        };
                    } else {
                        newPrototype[key] = value;
                    }
                }
            };

            unsequester = function (instance, fn, fnArgs) {
                // Unsequester all sequestered functions.
                for (var key in sequestered) {
                    if (hasOwn.call(sequestered, key) && instance[key] === replacements[key]) {
                        instance[key] = sequestered[key];
                    }
                }
                // Execute the supplier constructor.
                supplier.apply(instance, args);
                // Finally, execute the original sequestered function.
                return fn.apply(instance, fnArgs);
            };

            if (whitelist) {
                T.Array.each(whitelist, function (name) {
                    if (name in sProto) {
                        copy(sProto[name], name);
                    }
                });
            } else {
                T.Object.each(sProto, copy, null, true);
            }
        }
        T.mix(to, newPrototype || sProto, overwrite, whitelist);
        if (!sequester) {
            supplier.apply(to, args);
        }
        return receiver;
    };

    /* =============================== aggregate ================================== */
    
    /**
     * Applies object properties from the supplier to the receiver.  If
     * the target has the property, and the property is an object, the target
     * object will be augmented with the supplier's value.  If the property
     * is an array, the suppliers value will be appended to the target.
     * @method aggregate
     * @param {function} r  the object to receive the augmentation.
     * @param {function} s  the object that supplies the properties to augment.
     * @param {boolean} ov if true, properties already on the receiver
     * will be overwritten if found on the supplier.
     * @param {string[]} wl a whitelist.  If supplied, only properties in
     * this list will be applied to the receiver.
     * @return {object} the extended object.
     */
    T.aggregate = function(r, s, ov, wl) {
        return T.mix(r, s, ov, wl, 0, true);
    };

    /* =============================== extend ================================== */

    /**
     * Utility to set up the prototype, constructor and superclass properties to
     * support an inheritance strategy that can chain constructors and methods.
     * Static members will not be inherited.
     *
     * @method extend
     * @param {function} r   the object to modify.
     * @param {function} s the object to inherit.
     * @param {object} px prototype properties to add/override.
     * @param {object} sx static properties to add/override.
     * @return {object} the extended object.
     */
    T.extend = function(r, s, px, sx) {
        if (!s || !r) {
            T.error('extend failed, verify dependencies');
        }
        var sp = s.prototype;
        var rp = T.Object(sp);

        r.prototype = rp;
        rp.constructor = r;
        r.superclass = sp;
        // assign constructor property
        if (s != Object && sp.constructor == OP.constructor) {
            sp.constructor = s;
        }
        // add prototype overrides
        if (px) {
            T.mix(rp, px, true);
        }
        // add object overrides
        if (sx) {
            T.mix(r, sx, true);
        }
        return r;
    };

    /* =============================== each ================================== */

    /**
     * Executes the supplied function for each item in
     * a collection.  Supports arrays, objects, and
     * NodeLists
     * @method each
     * @param {object} o the object to iterate.
     * @param {function} f the function to execute.  This function
     * receives the value, key, and object as parameters.
     * @param {object} c the execution context for the function.
     * @param {boolean} proto if true, prototype properties are
     * iterated on objects.
     * @return {EM} the EM instance.
     */
    T.each = function(o, f, c, proto) {
        return dispatch(o, f, c, proto, 'each');
    };

    /* =============================== some ================================== */

    /**
     * Executes the supplied function for each item in
     * a collection.  The operation stops if the function
     * returns true. Supports arrays, objects, and
     * NodeLists.
     * @method some
     * @param {object} o the object to iterate.
     * @param {function} f the function to execute.  This function
     * receives the value, key, and object as parameters.
     * @param {object} c the execution context for the function.
     * @param {boolean} proto if true, prototype properties are
     * iterated on objects.
     * @return {boolean} true if the function ever returns true,
     * false otherwise.
     */
    T.some = function(o, f, c, proto) {
        return dispatch(o, f, c, proto, 'some');
    };

    T.log("module: "+name+" initialised!", "2.info", "inheritUtil.js");
}, "0.0.1", {});
