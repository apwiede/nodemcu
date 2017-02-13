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
 * File:   dwarfDbgErrorCodes.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#ifndef DWARFDBG_ERROR_CODES_H
#define	DWARFDBG_ERROR_CODES_H

#define checkErrOK(result) if (result != DWARF_DBG_ERR_OK) return result

enum dwarfDbgErrorCodes {
  DWARF_DBG_ERR_OK                                   = 0,
  DWARF_DBG_ERR_NO_ENTRY                             = 1,
  DWARF_DBG_ERR_OUT_OF_MEMORY                        = 2,
  DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT         = 3,
  DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT   = 4,
  DWRAF_DBG_ERR_GET_SRC_FILES                        = 5,
  DWARF_DBG_ERR_GET_SRC_LINES                        = 6,
  DWARF_DBG_ERR_GET_LINE_ADDR                        = 7,
  DWARF_DBG_ERR_GET_LINE_NO                          = 8,
  DWARF_DBG_ERR_NO_FILE_LINES                        = 9,
  DWARF_DBG_ERR_DW_TAG_STRING_NOT_FOUND              = 10,
  DWARF_DBG_ERR_DW_FORM_STRING_NOT_FOUND             = 11,
  DWARF_DBG_ERR_DW_AT_STRING_NOT_FOUND               = 12,
  DWARF_DBG_ERR_DW_ATE_STRING_NOT_FOUND              = 13,
  DWARF_DBG_ERR_DW_OP_STRING_NOT_FOUND               = 14,
  DWARF_DBG_ERR_CANNOT_GET_ADDR_SIZE                 = 15,
  DWARF_DBG_ERR_CANNOT_GET_LOC_LIST_C                = 16,
  DWARF_DBG_ERR_CANNOT_GET_LOC_DESC_ENTRY_C          = 17,
  DWARF_DBG_ERR_CANNOT_GET_LOCATION_OP_VALUE_C       = 18,
  DWARF_DBG_ERR_CANNOT_GET_FDE_LIST                  = 19,
  DWARF_DBG_ERR_CANNOT_GET_FDE_RANGE                 = 20,
  DWARF_DBG_ERR_GET_FDE_INFO_FOR_CFA_REG3_B          = 21,
  DWARF_DBG_ERR_GET_FDE_INFO_FOR_REG3                = 22,
  DWARF_DBG_ERR_MISSING_ATTR_IN_SWITCH               = 23,
  DWARF_DBG_ERR_CANNOT_GET_FORMUDATA                 = 24,
  DWARF_DBG_ERR_CANNOT_GET_WHATFORM                  = 25,
  DWARF_DBG_ERR_CANNOT_GET_WHATFORM_DIRECT           = 26,
  DWARF_DBG_ERR_CANNOT_GET_LANGUAGE_NAME             = 27,
  DWARF_DBG_ERR_CANNOT_GET_NAME_FORMSTRING           = 28,
  DWARF_DBG_ERR_CANNOT_GET_GLOBAL_FORMREF            = 29,
  DWARF_DBG_ERR_CANNOT_GET_CU_OFFSET                 = 30,
  DWARF_DBG_ERR_CANNOT_GET_DIE_OFFSETS               = 31,
  DWARF_DBG_ERR_CANNOT_GET_ATTR                      = 32,
  DWARF_DBG_ERR_CANNOT_GET_FORMADDR                  = 33,
  DWARF_DBG_ERR_CANNOT_GET_FORMREF                   = 34,
  DWARF_DBG_ERR_CANNOT_GET_FORMDATA                  = 35,
  DWARF_DBG_ERR_BAD_FRAME_BASE_FORM                  = 36,
  DWARF_DBG_ERR_CANNOT_GET_FORM_EXPRLOC              = 37,
  DWARF_DBG_ERR_GET_LOCLIST_FROM_EXPR_C              = 38,
  DWARF_DBG_ERR_GET_LOCDESC_ENTRY_C                  = 39,
  DWARF_DBG_ERR_GET_VERSION_OF_DIE                   = 40,
  DWARF_DBG_ERR_GET_DIE_ADDRESS_SIZE                 = 41,
};

#endif  /* DWARFDBG_ERROR_CODES_H */
