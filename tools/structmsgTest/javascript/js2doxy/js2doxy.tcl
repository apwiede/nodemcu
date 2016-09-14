#!/usr/bin/env tclsh8.6

#    [list {@.} AT1]\

set tokenPatterns [list \
    [list {\A\s+} WHITESPACE] \
    [list {\A\d+} NUMBER] \
    [list {\A0x[[:xdigit:]]+} HEX_NUMBER] \
    [list {\A[*][/]} COMMENT_END] \
    [list {\A/[*][*]} COMMENT_START_1] \
    [list {\A/[*][!]} COMMENT_START_2] \
    [list {\A[/][*]} COMMENT_START_3] \
    [list {\A//} COMMENT_START_4] \
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

proc parseLine {line} {
  set offset 0
  set lgth [string length $line]
puts stderr "LINE: $line!"
  while {$offset < $lgth} {
    foreach entry $::tokenPatterns {
      foreach {reg type} $entry break
#puts stderr "reg: $reg!"
      if {[regexp -start $offset $reg $line token]} {
#puts stderr "type: $type![string range $line $offset [expr {$offset + 1}]]!"
        switch $type {
          COMMENT_START_1 -
          COMMENT_START_2 -
          COMMENT_START_3 {
puts stderr "CSTART"
            if {!$::inComment} {
              set ::inComment true
              set ::onLineComment false
              set ::inText true
              set ::txt ""
            }
          }
          COMMENT_START_4 {
            if {!$::inComment} {
              set ::inComment true
              set ::onLineComment true
              set ::inText true
              set ::txt ""
            }
          }
        }
        if {$::inText} {
          append ::txt $token
#puts stderr "::txtx !$token!type: $type!offset: $offset!lgth: $lgth!"
        } else {
puts stderr "token: $token!$type!$offset!"
        }
        if {$type eq "COMMENT_END"} {
puts stderr "++++CEND"
          set ::inComment false
          set ::onLineComment false
puts stderr "::txt1: $::txt!"
          set ::txt ""
          set ::inText false
        }
        set offset [expr {$offset + [string length $token]}]
#puts stderr "offset: $offset!token: $token!"
        if {!$::inText} {
#puts stderr "offset2: $offset![string range $line $offset end]!"
        }
        if {$offset >= $lgth} {
          append ::txt "\n"
#puts stderr "::txt2 $::txt!"
          if {$::oneLineComment} {
            set ::inComment false
            set ::onLineComment false
            set ::inText false
          }
          break
        }
        break
      }
    }
  }
}

set fd stdin
while {![eof $fd]} {
  gets $fd line
  parseLine $line 
}

