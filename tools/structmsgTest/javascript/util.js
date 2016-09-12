/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg util for Esp (util.js)
 *
 * This code is mostly taken from:
 * http://yuilibrary.com/ YUI 3.3 version
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 *
 */

/* this are additional Object utility functions */
/* right now we only use "isObject" */

EM.addModule("Esp-util", function(T, name) {
    var U = T.Util || (T.Util = {});
    var STRING_PROTO = String.prototype;
    var TOSTRING     = Object.prototype.toString;
    var TYPES = {
        'undefined':         'undefined',
        'number':            'number',
        'boolean':           'boolean',
        'string':            'string',
        '[object Function]': 'function',
        '[object RegExp]':   'regexp',
        '[object Array]':    'array',
        '[object Date]':     'date',
        '[object Error]':    'error'
    };
    var SUBREGEX  = /\{\s*([^|}]+?)\s*(?:\|([^}]*))?\s*\}/g;
    var TRIMREGEX = /^\s+|\s+$/g;

    // If either MooTools or Prototype is on the page, then there's a chance that we
    // can't trust "native" language features to actually be native. When this is
    // the case, we take the safe route and fall back to our own non-native
    // implementation.
    var win           = T.config.win;
    var unsafeNatives = win && !!(win.MooTools || win.Prototype);

    /* =============================== U.isArray ================================== */

    /**
     * Determines whether or not the provided item is an array.
     *
     * Returns `false` for array-like collections such as the function `arguments`
     * collection or `HTMLElement` collections. Use `Y.Array.test()` if you want to
     * test for an array-like collection.
     *
     * @method isArray
     * @param o The object to test.
     * @return {boolean} true if o is an array.
     * @static
     */

    U.isArray = (!unsafeNatives && Array.isArray) || function (o) {
print("U.isArray");
        return U.type(o) === 'array';
    };

    /* =============================== U.isBoolean ================================== */

    /**
     * Determines whether or not the provided item is a boolean.
     * @method isBoolean
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is a boolean.
     */

    U.isBoolean = function(o) {
        return typeof o === 'boolean';
    };

    /* =============================== U.isFunction ================================== */

    /**
     * <p>
     * Determines whether or not the provided item is a function.
     * Note: Internet Explorer thinks certain functions are objects:
     * </p>
     *
     * <pre>
     * var obj = document.createElement("object");
     * U.Util.isFunction(obj.getAttribute) // reports false in IE
     * &nbsp;
     * var input = document.createElement("input"); // append to body
     * Y.Util.isFunction(input.focus) // reports false in IE
     * </pre>
     *
     * <p>
     * You will have to implement additional tests if these functions
     * matter to you.
     * </p>
     *
     * @method isFunction
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is a function.
     */

    U.isFunction = function(o) {
        return U.type(o) === 'function';
    };


    /* =============================== U.isDate ================================== */

    /**
     * Determines whether or not the supplied item is a date instance.
     * @method isDate
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is a date.
     */

    U.isDate = function(o) {
        return U.type(o) === 'date' && o.toString() !== 'Invalid Date' && !isNaN(o);
    };


    /* =============================== U.isNull ================================== */

    /**
     * Determines whether or not the provided item is null.
     * @method isNull
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is null.
     */

    U.isNull = function(o) {
        return o === null;
    };


    /* =============================== U.isNumber ================================== */

    /**
     * Determines whether or not the provided item is a legal number.
     * @method isNumber
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is a number.
     */

    U.isNumber = function(o) {
        return typeof o === 'number' && isFinite(o);
    };


    /* =============================== U.isObject ================================== */

    /**
     * Determines whether or not the provided item is of type object
     * or function. Note that arrays are also objects, so
     * <code>Y.Util.isObject([]) === true</code>.
     * @method isObject
     * @static
     * @param o The object to test.
     * @param failfn {boolean} fail if the input is a function.
     * @return {boolean} true if o is an object.
     * @see isPlainObject
     */

    U.isObject = function(o, failfn) {
        var t = typeof o;
        return(o && (t === 'object' || (!failfn && (t === 'function' || U.isFunction(o))))) || false;
    };


    /* =============================== U.isString ================================== */

    /**
     * Determines whether or not the provided item is a string.
     * @method isString
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is a string.
     */

    U.isString = function(o) {
        return typeof o === 'string';
    };


    /* =============================== U.isUndefined ================================== */

    /**
     * Determines whether or not the provided item is undefined.
     * @method isUndefined
     * @static
     * @param o The object to test.
     * @return {boolean} true if o is undefined.
     */

    U.isUndefined = function(o) {
        return typeof o === 'undefined';
    };


    /* =============================== U.trim ================================== */

    /**
     * Returns a string without any leading or trailing whitespace.  If
     * the input is not a string, the input will be returned untouched.
     * @method trim
     * @static
     * @param s {string} the string to trim.
     * @return {string} the trimmed string.
     */

    U.trim = STRING_PROTO.trim ? function(s) {
        return s && s.trim ? s.trim() : s;
    } : function (s) {
        try {
            return s.replace(TRIMREGEX, '');
        } catch (e) {
            return s;
        }
    };

    /* =============================== U.trimLeft ================================== */

    /**
     * Returns a string without any leading whitespace.
     * @method trimLeft
     * @static
     * @param s {string} the string to trim.
     * @return {string} the trimmed string.
     */

    U.trimLeft = STRING_PROTO.trimLeft ? function (s) {
        return s.trimLeft();
    } : function (s) {
        return s.replace(/^\s+/, '');
    };

    /* =============================== U.trimRight ================================== */
    
    /**
     * Returns a string without any trailing whitespace.
     * @method trimRight
     * @static
     * @param s {string} the string to trim.
     * @return {string} the trimmed string.
     */
    
    U.trimRight = STRING_PROTO.trimRight ? function (s) {
        return s.trimRight();
    } : function (s) {
        return s.replace(/\s+$/, '');
    };

    /* =============================== U.isValue ================================== */

    /**
     * A convenience method for detecting a legitimate non-null value.
     * Returns false for null/undefined/NaN, true for other values,
     * including 0/false/''
     * @method isValue
     * @static
     * @param o The item to test.
     * @return {boolean} true if it is not null/undefined/NaN || false.
     */

    U.isValue = function(o) {
        var t = U.type(o);
        switch (t) {
        case 'number':
            return isFinite(o);
        case 'null': // fallthru
        case 'undefined':
            return false;
        default:
            return !!t;
        }
    };

    /* =============================== U.type ================================== */

    /**
     * <p>
     * Returns a string representing the type of the item passed in.
     * </p>
     *
     * <p>
     * Known issues:
     * </p>
     *
     * <ul>
     *   <li>
     *     <code>typeof HTMLElementCollection</code> returns function in Safari, but
     *     <code>T.type()</code> reports object, which could be a good thing --
     *     but it actually caused the logic in <code>T.Util.isObject</code> to fail.
     *   </li>
     * </ul>
     *
     * @method type
     * @param o the item to test.
     * @return {string} the detected type.
     * @static
     */

    U.type = function(o) {
        return TYPES[typeof o] || TYPES[TOSTRING.call(o)] || (o ? 'object' : 'null');
    };

    /* =============================== U.sub ================================== */

    /**
     * Lightweight version of <code>Y.substitute</code>. Uses the same template
     * structure as <code>Y.substitute</code>, but doesn't support recursion,
     * auto-object coersion, or formats.
     * @method sub
     * @param {string} s String to be modified.
     * @param {object} o Object containing replacement values.
     * @return {string} the substitute result.
     * @static
     */

    U.sub = function(s, o) {
        return s.replace ? s.replace(SUBREGEX, function (match, key) {
            return U.isUndefined(o[key]) ? match : o[key];
        }) : s;
    };

    /* =============================== U.now ================================== */

    /**
     * Returns the current time in milliseconds.
     *
     * @method now
     * @return {Number} Current time in milliseconds.
     * @static
     */

    U.now = Date.now || function () {
        return new Date().getTime();
    };

    T.util = U;
    T.log("module: "+name+" initialised!", "2.info", "util.js");
}, '0.0.1', {});
