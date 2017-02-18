/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   dwarfDbgStringInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 05, 2017
 */

#ifndef DWARF_DBG_STRING_INFO_H
#define	DWARF_DBG_STRING_INFO_H

typedef struct id2Str {
  Dwarf_Half id;
  const char *str;
} id2Str_t;

typedef uint8_t (* getDW_TAG_string_t)(dwarfDbgPtr_t self, Dwarf_Half tag, const char **string);
typedef uint8_t (* getDW_FORM_string_t)(dwarfDbgPtr_t self, Dwarf_Half theform, const char **string);
typedef uint8_t (* getDW_AT_string_t)(dwarfDbgPtr_t self, Dwarf_Half attr, const char **string);
typedef uint8_t (* getDW_ATE_string_t)(dwarfDbgPtr_t self, Dwarf_Half attr, const char **string);
typedef uint8_t (* getDW_OP_string_t)(dwarfDbgPtr_t self, Dwarf_Small op, const char **string);
typedef uint8_t (* getDW_INL_string_t)(dwarfDbgPtr_t self, Dwarf_Unsigned inl, const char **string);
typedef uint8_t (* getDW_RANGES_TYPE_string_t)(dwarfDbgPtr_t self, int dwrType, const char **string);

typedef struct dwarfDbgStringInfo {
  getDW_TAG_string_t getDW_TAG_string;
  getDW_FORM_string_t getDW_FORM_string;
  getDW_AT_string_t getDW_AT_string;
  getDW_ATE_string_t getDW_ATE_string;
  getDW_OP_string_t getDW_OP_string;
  getDW_INL_string_t getDW_INL_string;
  getDW_RANGES_TYPE_string_t getDW_RANGES_TYPE_string;

} dwarfDbgStringInfo_t;

#endif  /* DWARF_DBG_STRING_INFO */
