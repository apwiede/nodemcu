#!/usr/bin/env tclsh8.6

#    [list {@.} AT1]\

set tokenPatterns [list \
    [list {\A\s+} WHITESPACE] \
    [list {\A\d+} NUMBER] \
    [list {\A0x[[:xdigit:]]+} HEX_NUMBER] \
    [list {\A[*][/]} COMMENT_END] \
    [list {\A/[*][*]} JS_DOC_COMMENT] \
    [list {\A/[*][!]} DOC_COMMENT] \
    [list {\A[/][*]} COMMENT] \
    [list {\A//} LINE_COMMENT] \
    [list {\A[*]} ASTERIKS] \
    [list {\A\w+} IDENTIFIER] \
    [list {\A[.]} DOT] \
    [list {.} CHAR] \
    [list {\A[\r\n\f]} EOL] \
]

set newlinePattern "\[\r\n\f\]"

set ::inText false
set ::inComment false
set ::oneLineComment false
set ::currLine ""
set ::commentType ""

proc parseLine {offset typeVar tokenVar} {
  upvar $typeVar tokenType
  upvar $tokenVar token

  set token ""
  set tokenType ""
  set lgth [string length $::currLine]
#puts stderr "parseLine:$::currLine!offset: $offset!inComment: $::inComment!oneLineComment: $::oneLineComment!"
  if {$offset >= $lgth} {
    set tokenType EOL
    set token ""
    if {$::inComment} {
      set tokenType COMMENT_EOL
      append ::txt ""
    }
#puts stderr "ret1 offset: $offset!"
    return $offset
  }
  while {$offset < $lgth} {
    foreach entry $::tokenPatterns {
      set tokenFound false
      foreach {reg type} $entry break
#puts stderr "reg: $reg!"
#puts stderr "offset: $offset!type: $type!"
      if {[regexp -start $offset $reg $::currLine token]} {
#puts stderr "type: $type!offset: $offset!lgth: $lgth!token: $token![string range $::currLine $offset [expr {$offset + 1}]]!"
        set tokenFound true
        switch $type {
          COMMENT -
          DOC_COMMENT -
          JS_DOC_COMMENT {
#puts stderr "CSTART"
            if {!$::inComment} {
              set ::inComment true
              set ::oneLineComment false
              set ::inText true
              set ::txt ""
              set ::commentType $type
              if {$type eq "JS_DOC_COMMENT"} {
                set relOffset [expr {$offset + [string length $token]}]
                if {!($relOffset >= $lgth)} {
                  if {[string range $::currLine $relOffset $relOffset] ne " "} {
                    # it is not a JS_DOC_COMMENT!!
                    set ::commentType COMMENT
                    set type COMMENT
                  }
                }
              }
            }
#puts stderr "COMM: $::commentType!"
          }
          LINE_COMMENT {
#puts stderr "start LINE_COMMENT: $::currLine!offset: $offset!"
            if {!$::inComment} {
              set ::inComment true
              set ::oneLineComment true
              set ::inText true
              set ::txt ""
              set ::commentType $type
            }
          }
        }
        set offset [expr {$offset + [string length $token]}]
        if {$::inText} {
          append ::txt $token
#puts stderr "::txtx !token: $token!type: $type!offset: $offset!lgth: $lgth!inComment: $::inComment!"
        } else {
#puts stderr "token: $type!$token!offset: $offset!"
          set tokenType $type
          set token $token
          return $offset
        }
#puts stderr "offset: $offset!token: $token!"
        if {$type eq "COMMENT_END"} {
#puts stderr "++++CEND"
#puts stderr "ret1 lgth $lgth offset $offset"
          if {$lgth >= $offset} {
            append ::txt "\n"
          }
          set tokenType $::commentType
          set token $::txt
          set ::inComment false
          set ::onLineComment false
#puts stderr "::txt1: $::txt!"
          set ::txt ""
          set ::inText false
#puts stderr "ret2 offset: $offset!lgth: $lgth!"
          return $offset
        }
        if {!$::inText} {
#puts stderr "offset2: $offset![string range $::currLine $offset end]!"
          set tokenType $type
          set token $token
#puts stderr "ret3 offset: $offset!"
          return $offset
        }
        if {$offset >= $lgth} {
#puts stderr "::txt2 $::txt!"
          if {$::oneLineComment} {
            set ::inComment false
            set ::oneLineComment false
            set ::inText false
            set tokenType $::commentType
            set token $::txt
#puts stderr "ret4 offset: $offset!"
            return $offset
          } else {
            if {$::inComment} {
              set tokenType COMMENT_EOL
              set token ""
              if {$::inText} {
                append ::txt "\n"
              }
#puts stderr "ret4a offset: $offset!"
              return $offset
            }
          }
          
        }
      }
      if {$tokenFound} {
        break
      } else {
if {$lgth == 0} {
puts stderr "emptyLine!"
}
#puts stderr "no token found: $offset!$lgth![string length $::currLine]!"
      }
    }
  }
}

set fd stdin
set isEof false

proc nextLine {} {
  gets $::fd ::currLine
  if {[eof $::fd]} {
    set ::isEof true
puts stderr "eof!"
    return ""
  }
#puts stderr "LINE: $::currLine!"
  return $::currLine
}

proc parse {} {
  nextLine
  set offset 0
  while {true} {
    set offset [parseLine $offset tokenType token]
puts stderr "token: $tokenType: $token!"
    if {($tokenType eq "EOL") || ($tokenType eq "COMMENT_EOL")} {
      set ::currLine [nextLine]
      set lgth [string length $::currLine]
      set offset 0
    }
    if {($offset < 0) || ($::isEof)} {
puts stderr "Eof $offset!$::isEof!"
      break
    }
  }
}

parse
puts stderr "END"
