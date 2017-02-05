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

# ==========================================================================
# This code is derived from modules in poApps:
# http://www.posoft.de/html/poTools.html
# Copyright:      Paul Obermeier 2013-2015 / paul@poSoft.de
# ==========================================================================

# ================================ showSourceFile ===============================

namespace eval showSourceFile {
    variable ns [namespace current]

    namespace ensemble create

    namespace export Init CreateScrolledTablelist CreateScrolledText

    # The following variables must be set, before reading parameters and
    # before calling LoadSettings.

    # ================================ Init ===============================

    proc Init {} {
        variable sApw

        set sApw(tw)      ".apwDef" ; # Name of toplevel window
        set sApw(appName) "apwDef"  ; # Name of tool
        set sApw(cfgDir)  ""        ; # Directory containing config files

        set sApw(startDir) [pwd]
        set sApw(dir1)     $sApw(startDir)
        set sApw(dir2)     $sApw(startDir)

        set sApw(infoWinList) {}         ; # Start with empty file info window list
        set sApw(leftFile)    ""         ; # Left  file for selective diff
        set sApw(rightFile)   ""         ; # Right file for selective diff
        set sApw(curSession)  "Default"  ; # Default session name
        set sApw(sessionList) [list]
        set sApw(curListbox)  ""

        # Default values for command line options.
        set sApw(optSync)             false
        set sApw(optSyncDelete)       false
        set sApw(optCopyDate)         false
        set sApw(optCopyDays)         ""
        set sApw(optSearch)           false
        set sApw(optConvert)          false
        set sApw(optConvertFmt)       ""
        set sApw(optDiffOnStartup)    false
        set sApw(optSessionOnStartup) ""
        # Add command line options which should not be expanded by file matching.
#        apwApps AddFileMatchIgnoreOption "filematch"
    }

    # ================================ CreateScrolledTablelist ===============================

    proc CreateScrolledTablelist { fr title } {
        set id [apwWin CreateScrolledTablelist $fr true $title \
                    -columns {50 "col1"   "left"
                               0 "col2"   "right"
                               0 "col3" "right" } \
                    -exportselection false \
                    -stretch 0 \
                    -stripebackground #e0e8f0 \
                    -selectmode extended \
                    -labelcommand tablelist::sortByColumn \
                    -showseparators true]
        $id columnconfigure 0 -sortmode dictionary
        $id columnconfigure 1 -sortmode integer
        $id columnconfigure 2 -sortmode dictionary
        return $id
    }

    # ================================ CreateScrolledText ===============================

    proc CreateScrolledText { fr title rows } {
        set textId [apwWin CreateScrolledText $fr true $title -wrap word -width 120 -height $rows]
        return $textId
    }

}


