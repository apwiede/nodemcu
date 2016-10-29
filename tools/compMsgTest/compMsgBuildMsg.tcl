# ===========================================================================
# * Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# *
# * 1. Redistributions of source code must retain the above copyright
# * notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# * notice, this list of conditions and the following disclaimer in the
# * documentation and/or other materials provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its
# * contributors may be used to endorse or promote products derived
# * from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

# buildMsgInfos dict
#  numEntries
#  type
#  parts
#  numRows
#  u8CmdKey
#  u16CmdKey
#  partsFlags
#  fieldNameStr
#  fieldValueStr
#  fieldNameId
#  fieldTypeId
#  tableRow
#  tableCol
#  value
#  buf[100]

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgBuildMsg

  namespace eval compMsgBuildMsg {
    namespace ensemble create
      
    namespace export setMsgValues buildMsgFromHeaderPart

    variable buildMsgInfos

    set buildMsgInfos [dict create]

    # ================================= setMsgValues ====================================

    proc setMsgValues {} {
      variable buildMsgInfos

      set fileName [format "CompVal%s.txt" [dict get $buildMsgInfos u16CmdKey]]
      set fd [open $fileName, "r"]
      gets $fd buffer
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
#        return $::COMP_DISP_ERR_BAD_FILE_CONTENTS
      set result [::compMsg cmdMsgDispatcher setMsgValuesFromLines $fd $numEntries $msgHandle $type]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      close $fd
      return $::COMP_MSG_ERR_OK
    }

    # ================================= buildMsgFromHeaderPart ====================================

    proc buildMsgFromHeaderPart {hdr} {
      variable buildMsgInfos

pdict $hdr
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgBuildMsg
} ; # namespace compMsg
