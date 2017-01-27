/*
  Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2013-2016 David Anderson. All Rights Reserved.
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

/*  dwarfDbgEsb.c
    extensible string buffer.

    A simple means (vaguely like a C++ class) that
    enables safely saving strings of arbitrary length built up
    in small pieces.

    We really do allow only C strings here. NUL bytes
    in a string result in adding only up to the NUL (and
    in the case of certain interfaces here a warning
    to stderr).

    The functions assume that
    pointer arguments of all kinds are not NULL.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "dwarfDbgInt.h"

/*  There is nothing magic about this size.
    It is just big enough to avoid most resizing. */
#define INITIAL_ALLOC 16
/*  Allow for final NUL */
static size_t allocSize = INITIAL_ALLOC;

// =============================== initEsbString ===========================

static uint8_t initEsbString(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t min_len) {
  char* d;

  if (data->esbAllocatedSize > 0) {
    return;
  }
  /* Only esbConstructor applied. Allow for string space. */
  if (min_len <= allocSize) {
    min_len = allocSize + 1;/* Allow for NUL at end */
  } else  {
    min_len++ ; /* Allow for NUL at end */
  }
  d = malloc(min_len);
  if (!d) {
    fprintf(stderr, "dwarfdump is out of memory allocating %lu bytes\n", (unsigned long) min_len);
    return TCL_ERROR;
  }
  data->esbString = d;
  data->esbAllocatedSize = min_len;
  data->esbString[0] = 0;
  data->esbUsedBytes = 0;
  return TCL_OK;
}

// =============================== esbAllocateMore ===========================

/*  Make more room. Leaving  contents unchanged, effectively.
  The NUL byte at end has room and this preserves that room.
*/
static uint8_t esbAllocateMore(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t len) {
  size_t new_size = data->esbAllocatedSize + len;
  char* newd = 0;

  if (new_size < allocSize) {
    new_size = allocSize;
  }
  newd = realloc(data->esbString, new_size);
  if (!newd) {
    fprintf(stderr, "dwarfdump is out of memory re-allocating %lu bytes\n", (unsigned long) new_size);
    return TCL_ERROR;
  }
  /*  If the area was reallocated by realloc() the earlier
    space was free()d by realloc(). */
  data->esbString = newd;
  data->esbAllocatedSize = new_size;
  return TCL_OK;
}

// =============================== esbForceAllocation ===========================

static uint8_t esbForceAllocation(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, size_t minlen) {
  if (data->esbAllocatedSize < minlen) {
    size_t increment = minlen - data->esbAllocatedSize;
    self->dwarfDbgEsb->esbAllocateMore(self, data,increment);
  }
  return TCL_OK;
}

// =============================== esbAppendn ===========================

static uint8_t esbAppendn(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string, size_t len) {
  size_t full_len = strlen(in_string);

  if (full_len < len) {
    fprintf(stderr, "dwarfdump esb internal error, bad string length "
      " %lu  < %lu \n",
      (unsigned long) full_len, (unsigned long) len);
    len = full_len;
  }

  self->dwarfDbgEsb->esbAppendnInternal(self, data, in_string, len);
  return TCL_OK;
}

// =============================== esbAppend ===========================

/*  The length is gotten from the in_string itself. */
static uint8_t esbAppend(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string) {
  size_t len = 0;
  if(in_string) {
    len = strlen(in_string);
    if (len) {
      self->dwarfDbgEsb->esbAppendnInternal(self, data, in_string, len);
    }
  }
  return TCL_OK;
}

// =============================== esbAppendnInternal ===========================

/*  The 'len' is believed. Do not pass in strings < len bytes long. */
static uint8_t esbAppendnInternal(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char * in_string, size_t len) {
  size_t remaining = 0;
  size_t needed = len;

  if (data->esbAllocatedSize == 0) {
    size_t maxlen = (len >= allocSize) ? (len) : allocSize;

    self->dwarfDbgEsb->initEsbString(self, data, maxlen);
  }
  /*  ASSERT: data->esbAllocatedSize > data->esbUsedBytes  */
  remaining = data->esbAllocatedSize - data->esbUsedBytes;
  if (remaining <= needed) {
    self->dwarfDbgEsb->esbAllocateMore(self, data,len);
  }
  strncpy(&data->esbString[data->esbUsedBytes], in_string, len);
  data->esbUsedBytes += len;
  /* Insist on explicit NUL terminator */
  data->esbString[data->esbUsedBytes] = 0;
  return TCL_OK;
}

// =============================== esbGetString ===========================

/*  Always returns an empty string or a non-empty string. Never 0. */
static char* esbGetString(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  if (data->esbAllocatedSize == 0) {
    self->dwarfDbgEsb->initEsbString(self, data, allocSize);
  }
  return data->esbString;
  return TCL_OK;
}

// =============================== esbEmptyString ===========================

/*  Sets esbUsedBytes to zero. The string is not freed and
  esbAllocatedSize is unchanged.  */
static uint8_t esbEmptyString(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  if (data->esbAllocatedSize == 0) {
    self->dwarfDbgEsb->initEsbString(self, data, allocSize);
  }
  data->esbUsedBytes = 0;
  data->esbString[0] = 0;
  return TCL_OK;
}

// =============================== esbStringLen ===========================

/*  Return esbUsed_bytes. */
static size_t esbStringLen(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  return data->esbUsedBytes;
}

// =============================== esbConstructor ===========================

/*  *data is presumed to contain garbage, not values, and
  is properly initialized here. */
static uint8_t esbConstructor(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  memset(data, 0, sizeof(*data));
  return TCL_OK;
}

// =============================== esbDestructor ===========================

/*  The string is freed, contents of *data set to zeroes. */
static uint8_t esbDestructor(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  if (data->esbString) {
    free(data->esbString);
    data->esbString = NULL;
  }
  self->dwarfDbgEsb->esbConstructor(self, data);
  return TCL_OK;
}

// =============================== esbAllocSize ===========================

/*  To get all paths in the code tested, this sets the
  allocation/reallocation to the given value, which can be quite small
  but must not be zero. */
static uint8_t esbAllocSize(dwarfDbgPtr_t self, size_t size) {
  allocSize = size;
  return TCL_OK;
}

// =============================== esbGetAllocatedSize ===========================

static size_t esbGetAllocatedSize(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  return data->esbAllocatedSize;
}

// =============================== esbAllocateMoreIfNeeded ===========================

/*  Make more room. Leaving  contents unchanged, effectively.
  The NUL byte at end has room and this preserves that room.
*/
static uint8_t esbAllocateMoreIfNeeded(dwarfDbgPtr_t self, dwarfDbgEsb_t *data, const char *in_string,va_list ap) {
#ifndef _WIN32
  static char a_buffer[512];
#endif /* _WIN32*/

  int netlen = 0;
  va_list ap_copy;

  /* Preserve the original argument list, to be used a second time */
  va_copy(ap_copy,ap);

  netlen = vsnprintf(a_buffer,sizeof(a_buffer),in_string,ap_copy);

  /*  "The object ap may be passed as an argument to another
    function; if that function invokes the va_arg()
    macro with parameter ap, the value of ap in the calling
    function is unspecified and shall be passed to the va_end()
    macro prior to any further reference to ap."
    Single Unix Specification. */
  va_end(ap_copy);

  /* Allocate enough space to hold the full text */
  self->dwarfDbgEsb->esbForceAllocation(self, data,netlen + 1);
  return TCL_OK;
}

// =============================== esbAppendPrintfAp ===========================

/*  Append a formatted string */
static uint8_t esbAppendPrintfAp(dwarfDbgPtr_t self, dwarfDbgEsb_t *data,const char *in_string,va_list ap) {
  int netlen = 0;
  int expandedlen = 0;

  /* Allocate enough space for the input string */
  self->dwarfDbgEsb->esbAllocateMoreIfNeeded(self, data,in_string,ap);

  netlen = data->esbAllocatedSize - data->esbUsedBytes;
  expandedlen = vsnprintf(&data->esbString[data->esbUsedBytes], netlen,in_string,ap);
  if (expandedlen < 0) {
    /*  There was an error.
      Do nothing. */
    return;
  }
  if (netlen < expandedlen) {
    /*  If data was too small, the max written was one less than
      netlen. */
    data->esbUsedBytes += netlen - 1;
  } else {
    data->esbUsedBytes += expandedlen;
  }
  return TCL_OK;
}

// =============================== esbAppendPrintf ===========================

/*  Append a formatted string */
static uint8_t esbAppendPrintf(dwarfDbgPtr_t self, dwarfDbgEsb_t *data,const char *in_string, ...) {
  va_list ap;
  va_start(ap,in_string);
  self->dwarfDbgEsb->esbAppendPrintfAp(self, data,in_string,ap);
  /*  "The object ap may be passed as an argument to another
    function; if that function invokes the va_arg()
    macro with parameter ap, the value of ap in the calling
    function is unspecified and shall be passed to the va_end()
    macro prior to any further reference to ap."
    Single Unix Specification. */
  va_end(ap);
  return TCL_OK;
}

// =============================== esbGetCopy ===========================

/*  Get a copy of the internal data buffer.
  It is up to the code calling this
  to free() the string using the
  pointer returned here. */
static char* esbGetCopy(dwarfDbgPtr_t self, dwarfDbgEsb_t *data) {
  char* copy = NULL;
  size_t len = self->dwarfDbgEsb->esbStringLen(self, data);
  if (len) {
    copy = (char*)malloc(len + 1);
    strcpy(copy,self->dwarfDbgEsb->esbGetString(self, data));
  }
  return copy;
}

// =============================== dwarfDbgEsbInit ===========================

int dwarfDbgEsbInit(dwarfDbgPtr_t self) {
  self->dwarfDbgEsb->initEsbString = &initEsbString;
  self->dwarfDbgEsb->esbAllocateMore = &esbAllocateMore;
  self->dwarfDbgEsb->esbAppendnInternal = &esbAppendnInternal;
  self->dwarfDbgEsb->esbAllocateMoreIfNeeded = &esbAllocateMoreIfNeeded;
  self->dwarfDbgEsb->esbAppend = &esbAppend;
  self->dwarfDbgEsb->esbAppendn = &esbAppendn;
  self->dwarfDbgEsb->esbGetString = &esbGetString;
  self->dwarfDbgEsb->esbEmptyString = &esbEmptyString;
  self->dwarfDbgEsb->esbStringLen = &esbStringLen;
  self->dwarfDbgEsb->esbConstructor = &esbConstructor;
  self->dwarfDbgEsb->esbForceAllocation = &esbForceAllocation;
  self->dwarfDbgEsb->esbDestructor = &esbDestructor;
  self->dwarfDbgEsb->esbAllocSize = &esbAllocSize;
  self->dwarfDbgEsb->esbGetAllocatedSize = &esbGetAllocatedSize;
  self->dwarfDbgEsb->esbAppendPrintf = &esbAppendPrintf;
  self->dwarfDbgEsb->esbAppendPrintfAp = &esbAppendPrintfAp;
  self->dwarfDbgEsb->esbGetCopy = &esbGetCopy; 
  return DWARF_DBG_ERR_OK;
}
