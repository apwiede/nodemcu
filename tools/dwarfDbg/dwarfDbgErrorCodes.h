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
  DWARF_DBG_ERR_OUT_OF_MEMORY                        = 1,
  DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT         = 2,
  DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT   = 3,
  DWRAF_DBG_ERR_GET_SRC_FILES                        = 4,
  DWARF_DBG_ERR_GET_SRC_LINES                        = 5,
  DWARF_DBG_ERR_GET_LINE_ADDR                        = 6,
  DWARF_DBG_ERR_GET_LINE_NO                          = 7,
  DWARF_DBG_ERR_NO_FILE_LINES                        = 8,
  DWARF_DBG_ERR_CANNOT_GET_CU_OFFSET                 = 9,
  DWARF_DBG_ERR_DW_TAG_STRING_NOT_FOUND              = 10,
  DWARF_DBG_ERR_DW_FORM_STRING_NOT_FOUND             = 11,
  DWARF_DBG_ERR_DW_AT_STRING_NOT_FOUND               = 12,
};

#endif  /* DWARFDBG_ERROR_CODES_H */
