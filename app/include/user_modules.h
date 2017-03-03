#ifndef __USER_MODULES_H__
#define __USER_MODULES_H__

#define LUA_USE_BUILTIN_STRING		// for string.xxx()
#define LUA_USE_BUILTIN_TABLE		// for table.xxx()
#define LUA_USE_BUILTIN_COROUTINE	// for coroutine.xxx()
#define LUA_USE_BUILTIN_MATH		// for math.xxx(), partially work
// #define LUA_USE_BUILTIN_IO 			// for io.xxx(), partially work

// #define LUA_USE_BUILTIN_OS			// for os.xxx(), not work
// #define LUA_USE_BUILTIN_DEBUG
#define LUA_USE_BUILTIN_DEBUG_MINIMAL // for debug.getregistry() and debug.traceback()

#ifndef LUA_CROSS_COMPILER

// The default configuration is designed to run on all ESP modules including the 512 KB modules like ESP-01 and only
// includes general purpose interface modules which require at most two GPIO pins.
// See https://github.com/nodemcu/nodemcu-firmware/pull/1127 for discussions.
// New modules should be disabled by default and added in alphabetical order.
#define LUA_USE_MODULES_COMPMSG
#define LUA_USE_MODULES_FILE
#define LUA_USE_MODULES_NET
#define LUA_USE_MODULES_RBOOT
#define LUA_USE_MODULES_TMR
#define LUA_USE_MODULES_UART
#define LUA_USE_MODULES_WIFI

#endif  /* LUA_CROSS_COMPILER */
#endif	/* __USER_MODULES_H__ */
