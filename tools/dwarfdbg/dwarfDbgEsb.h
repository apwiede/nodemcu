/*
    Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
    Portions Copyright 2011 David Anderson. All Rights Reserved.
    This program is free software; you can redistribute it and/or modify it
    under the terms of version 2 of the GNU General Public License as
    published by the Free Software Foundation.

    This program is distributed in the hope that it would be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Further, this software is distributed without any warranty that it is
    free of the rightful claim of any third person regarding infringement
    or the like.  Any license provided herein, whether implied or
    otherwise, applies only to this software file.  Patent licenses, if
    any, provided herein do not apply to combinations of this program with
    other software, or any other product whatsoever.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write the Free Software Foundation, Inc., 51
    Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.
*/

/* this part is "stolen" from dwarfdump for use with dwarfDbg
 * Arnulf Wiedemann <arnulf@wiedemann-pri.de>
 */

/* dwarfDbgEsb.h
  Extensible string buffer.
  A simple vaguely  object oriented extensible string buffer.

  The struct could be opaque here, but it seems ok to expose
  the contents: simplifies debugging.
*/

#ifndef DWARFDBG_ESB_H
#define DWARFDBG_ESB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dwarfDbgEsb dwarfDbgEsb_t;

/* string length taken from string itself. */
typedef uint8_t (* esbAppend_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string);

/* The 'len' is believed. Do not pass in strings < len bytes long. */
typedef uint8_t (* esbAppendn_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string, size_t len);

/* Always returns an empty string or a non-empty string. Never 0. */
typedef char* (* esbGetString_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);


/* Sets esb_used_bytes to zero. The string is not freed and
   esb_allocated_size is unchanged.  */
typedef uint8_t (* esbEmptyString_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);


/* Return esb_used_bytes. */
typedef size_t (* esbStringLen_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);

/* The following are for testing esb, not use by dwarfdump. */

/* *data is presumed to contain garbage, not values, and is properly initialized. */
typedef uint8_t (* esbConstructor_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);

typedef uint8_t (* esbForceAllocation_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t minlen);

/*  The string is freed, contents of *data set to zeroes. */
typedef uint8_t (* esbDestructor_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);

/* To get all paths in the code tested, this sets the
   allocation/reallocation to the given value, which can be quite small
   but must not be zero. */
typedef uint8_t (* esbAllocSize_t)(dwarfDbgPtr_t self, size_t size);
typedef size_t (* esbGetAllocatedSize_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);

/* Append a formatted string */
typedef uint8_t (* esbAppendPrintf_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char *format, ...);

/*  Append a formatted string. The 'ap' must be just-setup with
    va_start(ap,format)  and
    when esb_append_printf_ap returns the ap is used up
    and should not be touched. */
typedef uint8_t (* esbAppendPrintfAp_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char *format, va_list ap);

/* Get a copy of the internal data buffer */
typedef char* (* esbGetCopy_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data);

typedef uint8_t (* initEsbString_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t min_len);
typedef uint8_t (* esbAllocateMore_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t len);
typedef uint8_t (* esbAppendnInternal_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string, size_t len);
typedef uint8_t (* esbAllocateMoreIfNeeded_t)(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char *in_string,va_list ap);

typedef struct dwarfDbgEsb {
  char *  esbString;        /* pointer to the data itself, or  NULL. */
  size_t  esbAllocatedSize; /* Size of allocated data or 0 */
  size_t  esbUsedBytes;     /* Amount of space used  or 0 */

  initEsbString_t initEsbString;
  esbAllocateMore_t esbAllocateMore;
  esbAppendnInternal_t esbAppendnInternal;
  esbAllocateMoreIfNeeded_t esbAllocateMoreIfNeeded;
  esbAppend_t esbAppend;
  esbAppendn_t esbAppendn;
  esbGetString_t esbGetString;
  esbEmptyString_t esbEmptyString;
  esbStringLen_t esbStringLen;
  esbConstructor_t esbConstructor;
  esbForceAllocation_t esbForceAllocation;
  esbDestructor_t esbDestructor;
  esbAllocSize_t esbAllocSize;
  esbGetAllocatedSize_t esbGetAllocatedSize;
  esbAppendPrintf_t esbAppendPrintf;
  esbAppendPrintfAp_t esbAppendPrintfAp;
  esbGetCopy_t esbGetCopy; 
} dwarfDbgEsb_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* DWARFDBG_ESB_H */
