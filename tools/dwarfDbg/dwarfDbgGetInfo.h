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
 * File:   dwarfDbgGetInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 24, 2017
 */

#ifndef DWARFDBG_GET_INFO_H
#define	DWARFDBG_GET_INFO_H

typedef struct compileUnit {
    Dwarf_Unsigned compileUnitHeaderLength;
    Dwarf_Half versionStamp;
    Dwarf_Off abbrevOffset;
    Dwarf_Half addressSize;
    Dwarf_Half lengthSize;
    Dwarf_Half extensionSize;
    Dwarf_Sig8 signature;
    Dwarf_Unsigned typeOffset;
    Dwarf_Unsigned nextCompileUnitOffset;
    Dwarf_Half compileUnitType;
} compileUnit_t;

typedef uint8_t (* addCompileUnit_t)(dwarfDbgPtr_t self, size_t *compileUnitIdx);
typedef uint8_t (* handleOneDieSection_t)(dwarfDbgPtr_t self);

typedef struct dwarfDbgGetInfo {
  size_t maxCompileUnit;
  size_t numCompileUnit;
  compileUnit_t *compileUnits;

  addCompileUnit_t addCompileUnit;
  handleOneDieSection_t handleOneDieSection;
} dwarfDbgGetInfo_t;

#endif  /* DWARFDBG_GET_INFO */
