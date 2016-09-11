/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg object for Esp (object.js)
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

/* this are additional Object utility functions */

EM.addModule("Esp-object", function(T, name) {
    /**
     * Adds utilities to the RAPL instance for working with objects.
     *
     * @class Object
     */

    var hasOwn = Object.prototype.hasOwnProperty,

    // If either MooTools or Prototype is on the page, then there's a chance that we
    // can't trust "native" language features to actually be native. When this is
    // the case, we take the safe route and fall back to our own non-native
    // implementations.
    win           = T.config.win,
    unsafeNatives = win && !!(win.MooTools || win.Prototype),

    UNDEFINED, // <-- Note the comma. We're still declaring vars.

    /* =============================== T.Object ================================== */

    /**
     * Returns a new object that uses _obj_ as its prototype. This method wraps the
     * native ES5 `Object.create()` method if available, but doesn't currently
     * pass through `Object.create()`'s second argument (properties) in order to
     * ensure compatibility with older browsers.
     *
     * @method ()
     * @param {Object} obj Prototype object.
     * @return {Object} New object using _obj_ as its prototype.
     * @static
     */

    O = T.Object = (!unsafeNatives && Object.create) ? function (obj) {
        // We currently wrap the native Object.create instead of simply aliasing it
        // to ensure consistency with our fallback shim, which currently doesn't
        // support Object.create()'s second argument (properties). Once we have a
        // safe fallback for the properties arg, we can stop wrapping
        // Object.create().
        return Object.create(obj);
    } : (function () {
        // Reusable constructor function for the Object.create() shim.
        function F() {}

        // The actual shim.
        return function (obj) {
            F.prototype = obj;
            return new F();
        };
    }()),

    /* =============================== O._forceEnum ================================== */

    /**
     * Property names that IE doesn't enumerate in for..in loops, even when they
     * should be enumerable. When `_hasEnumBug` is `true`, it's necessary to
     * manually enumerate these properties.
     *
     * @property _forceEnum
     * @type String[]
     * @protected
     * @static
     */

    forceEnum = O._forceEnum = [
        'hasOwnProperty',
        'isPrototypeOf',
        'propertyIsEnumerable',
        'toString',
        'toLocaleString',
        'valueOf'
    ],

    /* =============================== O._hasEnumBug ================================== */

    /**
     * `true` if this browser has the JScript enumeration bug that prevents
     * enumeration of the properties named in the `_forceEnum` array, `false`
     * otherwise.
     *
     * See:
     *   - <https://developer.mozilla.org/en/ECMAScript_DontEnum_attribute#JScript_DontEnum_Bug>
     *   - <http://whattheheadsaid.com/2010/10/a-safer-object-keys-compatibility-implementation>
     *
     * @property _hasEnumBug
     * @type {Boolean}
     * @protected
     * @static
     */

    hasEnumBug = O._hasEnumBug = !{valueOf: 0}.propertyIsEnumerable('valueOf'),

    /* =============================== O.owns ================================== */
    
    /**
     * Returns `true` if _key_ exists on _obj_, `false` if _key_ doesn't exist or
     * exists only on _obj_'s prototype. This is essentially a safer version of
     * `obj.hasOwnProperty()`.
     *
     * @method owns
     * @param {Object} obj Object to test.
     * @param {String} key Property name to look for.
     * @return {Boolean} `true` if _key_ exists on _obj_, `false` otherwise.
     * @static
     */

    owns = O.owns = function (obj, key) {
        return !!obj && hasOwn.call(obj, key);
    }; // <-- End of var declarations.

    /* =============================== O.hasKey ================================== */

    /**
     * Alias for `owns()`.
     *
     * @method hasKey
     * @param {Object} obj Object to test.
     * @param {String} key Property name to look for.
     * @return {Boolean} `true` if _key_ exists on _obj_, `false` otherwise.
     * @static
     */

    O.hasKey = owns;

    /* =============================== O.keys ================================== */

    /**
     * Returns an array containing the object's enumerable keys. Does not include
     * prototype keys or non-enumerable keys.
     *
     * Note that keys are returned in enumeration order (that is, in the same order
     * that they would be enumerated by a `for-in` loop), which may not be the same
     * as the order in which they were defined.
     *
     * This method is an alias for the native ES5 `Object.keys()` method if
     * available.
     *
     * @example
     *
     *     T.Object.keys({a: 'foo', b: 'bar', c: 'baz'});
     *     // => ['a', 'b', 'c']
     *
     * @method keys
     * @param {Object} obj An object.
     * @return {String[]} Array of keys.
     * @static
     */

    O.keys = (!unsafeNatives && Object.keys) || function (obj) {
        if (!T.Util.isObject(obj)) {
            throw new TypeError('Object.keys called on a non-object');
        }

        var keys = [];
        var i;
        var key;
        var len;

        for (key in obj) {
            if (owns(obj, key)) {
                keys.push(key);
            }
        }
        if (hasEnumBug) {
            for (i = 0, len = forceEnum.length; i < len; ++i) {
                key = forceEnum[i];
                if (owns(obj, key)) {
                    keys.push(key);
                }
            }
        }
        return keys;
    };

    /* =============================== O.values ================================== */

    /**
     * Returns an array containing the values of the object's enumerable keys.
     *
     * Note that values are returned in enumeration order (that is, in the same
     * order that they would be enumerated by a `for-in` loop), which may not be the
     * same as the order in which they were defined.
     *
     * @example
     *
     *     T.Object.values({a: 'foo', b: 'bar', c: 'baz'});
     *     // => ['foo', 'bar', 'baz']
     *
     * @method values
     * @param {Object} obj An object.
     * @return {Array} Array of values.
     * @static
     */

    O.values = function (obj) {
        var keys   = O.keys(obj);
        var i      = 0;
        var len    = keys.length;
        var values = [];

        for (; i < len; ++i) {
            values.push(obj[keys[i]]);
        }
        return values;
    };

    /* =============================== O.size ================================== */

    /**
     * Returns the number of enumerable keys owned by an object.
     *
     * @method size
     * @param {Object} obj An object.
     * @return {Number} The object's size.
     * @static
     */

    O.size = function (obj) {
        return O.keys(obj).length;
    };

    /* =============================== O.hasValue ================================== */
    
    /**
     * Returns `true` if the object owns an enumerable property with the specified
     * value.
     *
     * @method hasValue
     * @param {Object} obj An object.
     * @param {any} value The value to search for.
     * @return {Boolean} `true` if _obj_ contains _value_, `false` otherwise.
     * @static
     */
    
    O.hasValue = function (obj, value) {
        return T.Array.indexOf(O.values(obj), value) > -1;
    };

    /* =============================== O.each ================================== */

    /**
     * Executes a function on each enumerable property in _obj_. The function
     * receives the value, the key, and the object itself as parameters (in that
     * order).
     *
     * By default, only properties owned by _obj_ are enumerated. To include
     * prototype properties, set the _proto_ parameter to `true`.
     *
     * @method each
     * @param {Object} obj Object to enumerate.
     * @param {Function} fn Function to execute on each enumerable property.
     *   @param {mixed} fn.value Value of the current property.
     *   @param {String} fn.key Key of the current property.
     *   @param {Object} fn.obj Object being enumerated.
     * @param {Object} [thisObj] `this` object to use when calling _fn_.
     * @param {Boolean} [proto=false] Include prototype properties.
     * @return {AT} the AT instance.
     * @chainable
     * @static
     */

    O.each = function (obj, fn, thisObj, proto) {
        var key;

        for (key in obj) {
            if (proto || owns(obj, key)) {
                fn.call(thisObj || R, obj[key], key, obj);
            }
        }
        return R;
    };

    /* =============================== O.some ================================== */

    /**
     * Executes a function on each enumerable property in _obj_, but halts if the
     * function returns a truthy value. The function receives the value, the key,
     * and the object itself as paramters (in that order).
     *
     * By default, only properties owned by _obj_ are enumerated. To include
     * prototype properties, set the _proto_ parameter to `true`.
     *
     * @method some
     * @param {Object} obj Object to enumerate.
     * @param {Function} fn Function to execute on each enumerable property.
     *   @param {mixed} fn.value Value of the current property.
     *   @param {String} fn.key Key of the current property.
     *   @param {Object} fn.obj Object being enumerated.
     * @param {Object} [thisObj] `this` object to use when calling _fn_.
     * @param {Boolean} [proto=false] Include prototype properties.
     * @return {Boolean} `true` if any execution of _fn_ returns a truthy value,
     *   `false` otherwise.
     * @static
     */

    O.some = function (obj, fn, thisObj, proto) {
        var key;

        for (key in obj) {
            if (proto || owns(obj, key)) {
                if (fn.call(thisObj || R, obj[key], key, obj)) {
                    return true;
                }
            }
        }
        return false;
    };

    /* =============================== O.getValue ================================== */

    /**
     * Retrieves the sub value at the provided path,
     * from the value object provided.
     *
     * @method getValue
     * @static
     * @param o The object from which to extract the property value.
     * @param path {Array} A path array, specifying the object traversal path
     * from which to obtain the sub value.
     * @return {Any} The value stored in the path, undefined if not found,
     * undefined if the source is not an object.  Returns the source object
     * if an empty path is provided.
     */

    O.getValue = function(o, path) {
        if (!T.Util.isObject(o)) {
            return UNDEFINED;
        }

        var i;
        var p = T.Array(path);
        var l = p.length;
    
        for (i = 0; o !== UNDEFINED && i < l; i++) {
            o = o[p[i]];
        }
        return o;
    };

    /* =============================== O.setValue ================================== */

    /**
     * Sets the sub-attribute value at the provided path on the
     * value object.  Returns the modified value object, or
     * undefined if the path is invalid.
     *
     * @method setValue
     * @static
     * @param o             The object on which to set the sub value.
     * @param path {Array}  A path array, specifying the object traversal path
     *                      at which to set the sub value.
     * @param val {Any}     The new value for the sub-attribute.
     * @return {Object}     The modified object, with the new sub value set, or
     *                      undefined, if the path was invalid.
     */

    O.setValue = function(o, path, val) {
        var i;
        var p = T.Array(path);
        var leafIdx = p.length - 1;
        var ref = o;

        if (leafIdx >= 0) {
            for (i = 0; ref !== UNDEFINED && i < leafIdx; i++) {
                ref = ref[p[i]];
            }
            if (ref !== UNDEFINED) {
                ref[p[i]] = val;
            } else {
                return UNDEFINED;
            }
        }
        return o;
    };

    /* =============================== O.isEmpty ================================== */

    /**
     * Returns `true` if the object has no enumerable properties of its own.
     *
     * @method isEmpty
     * @param {Object} obj An object.
     * @return {Boolean} `true` if the object is empty.
     * @static
     * @since 3.2.0
     */

    O.isEmpty = function (obj) {
        return !O.keys(obj).length;
    };

    T.log("module: "+name+" initialised!", "2.info", "object.js");
}, '0.0.1', {});

