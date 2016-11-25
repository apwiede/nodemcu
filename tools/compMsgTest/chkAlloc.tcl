#!/usr/bin/env tclsh8.6

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

source pdict.tcl

set allocs [dict create]
set frees [dict create]
set reallocs [dict create]

while {[gets stdin line] >= 0} {
  set flds [split $line " "]
  foreach {type dummy1 addr} $flds break
  set info [lrange $flds 3 end]
  switch $type {
    os_zalloc: {
      dict set allocs $addr $info
    }
    os_free: {
      if {[dict exists $allocs $addr]} {
#puts stderr "found zalloc: $addr"
        set allocs [dict remove $allocs $addr]
      } else {
        if {[dict exists $reallocs $addr]} {
puts stderr "free found realloc: $addr"
          set reallocs [dict remove $allocs $addr]
        } else {
          dict set frees $addr $info
        }
      }
    }
    os_realloc: {
      foreach {dummy old_addr} $info break
      if {[dict exists $allocs $old_addr]} {
puts stderr "realloc found zalloc: $addr"
        set allocs [dict remove $allocs $old_addr]
      }
      dict set reallocs $addr $info
    }
    default {
      puts stderr "funny type: $type!flds: $flds!"
    }
  } 
}
puts stderr "zallocs:"
pdict $allocs
puts stderr "frees:"
pdict $frees
puts stderr "reallocs:"
pdict $reallocs
