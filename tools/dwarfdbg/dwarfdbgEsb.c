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

/* this part is "stolen" from dwarfdump for use with dwarfddbg 
 * Arnulf Wiedemann <arnulf@wiedemann-pri.de>
 */
/*  esb.c
    extensible string buffer.

    A simple means (vaguely like a C++ class) that
    enables safely saving strings of arbitrary length built up
    in small pieces.

    We really do allow only C strings here. NUL bytes
    in a string result in adding only up to the NUL (and
    in the case of certain interfaces here a warning
    to stderr).

    Do selftest as follows:
        gcc -DSELFTEST esb.c
        ./a.out
        valgrind --leak-check=full ./a.out

    The functions assume that
    pointer arguments of all kinds are not NULL.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "esb.h"

/*  There is nothing magic about this size.
    It is just big enough to avoid most resizing. */
#define INITIAL_ALLOC 16
/*  Allow for final NUL */
static size_t alloc_size = INITIAL_ALLOC;

/* NULL device used when printing formatted strings */
static FILE *null_device_handle = 0;
#define NULL_DEVICE_NAME "/dev/null"

/* Open the null device used during formatting printing */
FILE *esb_open_null_device(void)
{
    if (!null_device_handle) {
        null_device_handle = fopen(NULL_DEVICE_NAME,"w");
    }
    return null_device_handle;
}

/* Close the null device used during formatting printing */
void esb_close_null_device(void)
{
    if (null_device_handle) {
        fclose(null_device_handle);
    }
}

static void
init_esb_string(struct esb_s *data, size_t min_len)
{
    char* d;

    if (data->esb_allocated_size > 0) {
        return;
    }
    /* Only esb_constructor applied. Allow for string space. */
    if (min_len <= alloc_size) {
        min_len = alloc_size +1;/* Allow for NUL at end */
    } else  {
        min_len++ ; /* Allow for NUL at end */
    }
    d = malloc(min_len);
    if (!d) {
        fprintf(stderr,
            "dwarfdump is out of memory allocating %lu bytes\n",
            (unsigned long) min_len);
        exit(5);
    }
    data->esb_string = d;
    data->esb_allocated_size = min_len;
    data->esb_string[0] = 0;
    data->esb_used_bytes = 0;
}

/*  Make more room. Leaving  contents unchanged, effectively.
    The NUL byte at end has room and this preserves that room.
*/
static void
esb_allocate_more(struct esb_s *data, size_t len)
{
    size_t new_size = data->esb_allocated_size + len;
    char* newd = 0;

    if (new_size < alloc_size) {
        new_size = alloc_size;
    }
    newd = realloc(data->esb_string, new_size);
    if (!newd) {
        fprintf(stderr, "dwarfdump is out of memory re-allocating "
            "%lu bytes\n", (unsigned long) new_size);
        exit(5);
    }
    /*  If the area was reallocated by realloc() the earlier
        space was free()d by realloc(). */
    data->esb_string = newd;
    data->esb_allocated_size = new_size;
}

void
esb_force_allocation(struct esb_s *data, size_t minlen)
{
    if (data->esb_allocated_size < minlen) {
        size_t increment = minlen - data->esb_allocated_size;
        esb_allocate_more(data,increment);
    }
}

static void
esb_appendn_internal(struct esb_s *data, const char * in_string, size_t len);

void
esb_appendn(struct esb_s *data, const char * in_string, size_t len)
{
    size_t full_len = strlen(in_string);

    if (full_len < len) {
        fprintf(stderr, "dwarfdump esb internal error, bad string length "
            " %lu  < %lu \n",
            (unsigned long) full_len, (unsigned long) len);
        len = full_len;
    }

    esb_appendn_internal(data, in_string, len);
}

/*  The length is gotten from the in_string itself. */
void
esb_append(struct esb_s *data, const char * in_string)
{
    size_t len = 0;
    if(in_string) {
        len = strlen(in_string);
        if (len) {
            esb_appendn_internal(data, in_string, len);
        }
    }
}

/*  The 'len' is believed. Do not pass in strings < len bytes long. */
static void
esb_appendn_internal(struct esb_s *data, const char * in_string, size_t len)
{
    size_t remaining = 0;
    size_t needed = len;

    if (data->esb_allocated_size == 0) {
        size_t maxlen = (len >= alloc_size)? (len):alloc_size;

        init_esb_string(data, maxlen);
    }
    /*  ASSERT: data->esb_allocated_size > data->esb_used_bytes  */
    remaining = data->esb_allocated_size - data->esb_used_bytes;
    if (remaining <= needed) {
        esb_allocate_more(data,len);
    }
    strncpy(&data->esb_string[data->esb_used_bytes], in_string, len);
    data->esb_used_bytes += len;
    /* Insist on explicit NUL terminator */
    data->esb_string[data->esb_used_bytes] = 0;
}

/*  Always returns an empty string or a non-empty string. Never 0. */
char*
esb_get_string(struct esb_s *data)
{
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    return data->esb_string;
}


/*  Sets esb_used_bytes to zero. The string is not freed and
    esb_allocated_size is unchanged.  */
void
esb_empty_string(struct esb_s *data)
{
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    data->esb_used_bytes = 0;
    data->esb_string[0] = 0;
}


/*  Return esb_used_bytes. */
size_t
esb_string_len(struct esb_s *data)
{
    return data->esb_used_bytes;
}

/*  *data is presumed to contain garbage, not values, and
    is properly initialized here. */
void
esb_constructor(struct esb_s *data)
{
    memset(data, 0, sizeof(*data));
}

/*  The string is freed, contents of *data set to zeroes. */
void
esb_destructor(struct esb_s *data)
{
    if (data->esb_string) {
        free(data->esb_string);
        data->esb_string = 0;
    }
    esb_constructor(data);
}


/*  To get all paths in the code tested, this sets the
    allocation/reallocation to the given value, which can be quite small
    but must not be zero. */
void
esb_alloc_size(size_t size)
{
    alloc_size = size;
}

size_t
esb_get_allocated_size(struct esb_s *data)
{
    return data->esb_allocated_size;
}

/*  Make more room. Leaving  contents unchanged, effectively.
    The NUL byte at end has room and this preserves that room.
*/
static void
esb_allocate_more_if_needed(struct esb_s *data,
    const char *in_string,va_list ap)
{
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
    esb_force_allocation(data,netlen + 1);
}

/*  Append a formatted string */
void
esb_append_printf_ap(struct esb_s *data,const char *in_string,va_list ap)
{
    int netlen = 0;
    int expandedlen = 0;

    /* Allocate enough space for the input string */
    esb_allocate_more_if_needed(data,in_string,ap);

    netlen = data->esb_allocated_size - data->esb_used_bytes;
    expandedlen =
        vsnprintf(&data->esb_string[data->esb_used_bytes],
        netlen,in_string,ap);
    if (expandedlen < 0) {
        /*  There was an error.
            Do nothing. */
        return;
    }
    if (netlen < expandedlen) {
        /*  If data was too small, the max written was one less than
            netlen. */
        data->esb_used_bytes += netlen - 1;
    } else {
        data->esb_used_bytes += expandedlen;
    }
}

/*  Append a formatted string */
void
esb_append_printf(struct esb_s *data,const char *in_string, ...)
{
    va_list ap;
    va_start(ap,in_string);
    esb_append_printf_ap(data,in_string,ap);
    /*  "The object ap may be passed as an argument to another
        function; if that function invokes the va_arg()
        macro with parameter ap, the value of ap in the calling
        function is unspecified and shall be passed to the va_end()
        macro prior to any further reference to ap."
        Single Unix Specification. */
    va_end(ap);
}

/*  Get a copy of the internal data buffer.
    It is up to the code calling this
    to free() the string using the
    pointer returned here. */
char*
esb_get_copy(struct esb_s *data)
{
    char* copy = NULL;
    size_t len = esb_string_len(data);
    if (len) {
        copy = (char*)malloc(len + 1);
        strcpy(copy,esb_get_string(data));
    }
    return copy;
}

