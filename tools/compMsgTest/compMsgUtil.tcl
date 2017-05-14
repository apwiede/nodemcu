# ===========================================================================
# * Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgUtil

  namespace eval compMsgUtil {
    namespace ensemble create
      
    namespace export toBase64 fromBase64 encryptMsg decryptMsg addFieldDescription

    # ============================= toBase64 ========================
    
    proc toBase64 {msg len encoded} {
      n = *len
      if {!n} { # handle empty string case 
        checkErrOK OUT_OF_MEMORY
      }
      out = {uint8_t *}os_zalloc((n + 2) / 3 * 4);
      checkAllocOK{out};
      c_memcpy{bytes64, b64, sizeof(b64});   #Avoid lots of flash unaligned fetches
      
      for {i = 0, q = out; i < n; i += 3} {
        int a = msg[i];
        int b = {i + 1 < n} ? msg[i + 1] : 0;
        int c = {i + 2 < n} ? msg[i + 2] : 0;
        *q++ = bytes64[a >> 2];
        *q++ = bytes64[{(a & 3} << 4) | (b >> 4)];
        *q++ = {i + 1 < n} ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
        *q++ = {i + 2 < n} ? bytes64[(c & 63)] : BASE64_PADDING;
      }
      *len = q - out;
      *encoded = out;
      return [checkErrOK OK]
    }
    
    # ============================= fromBase64 ========================
    
    proc fromBase64 {encodedMsg len decodedMsg} {
      n = *len;
      blocks = {n>>2};
      pad = 0;
      if {!n} { # handle empty string case 
        checkErrOK OUT_OF_MEMORY
      } 
      if {n & 3} {
        checkErrOK INVALID_BASE64_STRING
      } 
      c_memset{unbytes64, BASE64_INVALID, sizeof(unbytes64});
      for {i = 0; i < sizeof(b64}-1; i++) {
        unbytes64[b64[i]] = i;  # sequential so no exceptions 
      }
      if {encodedMsg[n-1] == BASE64_PADDING} {
        pad =  {encodedMsg[n-2] != BASE64_PADDING} ? 1 : 2;
        blocks--;  #exclude padding block
      }    
    
      for {i = 0; i < n - pad; i++} {
        if {!ISBASE64(encodedMsg[i]}) {
          checkErrOK INVALID_BASE64_STRING
        }
      }
      unbytes64[BASE64_PADDING] = 0;
      q = {uint8_t *} os_zalloc(1+ (3 * n / 4)); 
      checkAllocOK{q};
      msg = q;
      for {i = 0, p = encodedMsg; i<blocks; i++} {
        uint8 a = unbytes64[*p++]; 
        uint8 b = unbytes64[*p++]; 
        uint8 c = unbytes64[*p++]; 
        uint8 d = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        *q++ = {b << 4} | (c >> 2);
        *q++ = {c << 6} | d;
      }
      if {pad} { #now process padding block bytes
        uint8 a = unbytes64[*p++];
        uint8 b = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        if {pad == 1} *q++ = (b << 4) | (unbytes64[*p] >> 2);
      }
      *len = q - msg;
      *decodedMsg = msg;
      return [checkErrOK OK]
    }
    
    # ============================= encryptMsg ========================
    
    proc encryptMsg {msg mlen key klen iv ivlen dataVar lgthVar} {
      upvar $dataVar data
      upvar $lgthVar lgth

      set data [aes::aes -mode cbc -dir encrypt -key $key -iv $iv $msg]
      set lgth [string length $data]
      return [checkErrOK OK]
    }
    
    # ============================= decryptMsg ========================
    
    proc decryptMsg {msg mlen key klen iv ivlen bufVar lgthVar} {
      upvar $bufVar buf
      upvar $lgthVar lgth

      set buf [aes::aes -mode cbc -dir decrypt -key $key -iv $iv $msg]
      set lgth [string length $buf]
      return [checkErrOK OK]
    }
    
    # ================================= addFieldDescription ====================================

    proc addFieldDescription {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgDescriptionInfos [dict get $compMsgMsgDesc msgDescriptionInfos]
#puts stderr "num: [dict get $msgDescriptionInfos numMsgDescriptions]!max: [dict get $msgDescriptionInfos maxMsgDescriptions]!"
#pdict msgDescriptionInfos
      if {[dict get $msgDescriptionInfos numMsgDescriptions] >= [dict get $msgDescriptionInfos maxMsgDescriptions]} {
        set msgDescription [dict create]
        dict set msgDescription headerFieldValues [list]
        dict set msgDescription encrypted N
        dict set msgDescription handleType ""
        dict set msgDescription cmdKey ""
        dict set msgDescription fieldSequence [list]
        if {[dict get $msgDescriptionInfos maxMsgDescriptions] == 0} {
          dict set msgDescriptionInfos maxMsgDescriptions 5
          set idx 0
          set lst [list]
          while {$idx < [dict get $msgDescriptionInfos maxMsgDescriptions]} {
            lappend lst $msgDescription
            incr idx
          }
          dict set msgDescriptionInfos msgDescriptions $lst
        } else {
          set numEntries 5
          dict incr msgDescriptionInfos maxMsgDescriptions $numEntries
	  set msgDescriptions [dict get $msgDescriptionInfos msgDescriptions]
          set idx 0
          while {$idx < $numEntries} {
            lappend msgDescriptions $msgDescription
            incr idx
          }
          dict set msgDescriptionInfos msgDescriptions $msgDescriptions
        }
      }
      dict incr msgDescriptionInfos numMsgDescriptions
      dict set compMsgMsgDesc msgDescriptionInfos $msgDescriptionInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

  } ; # namespace compMsgUtil
} ; # namespace compMsg
