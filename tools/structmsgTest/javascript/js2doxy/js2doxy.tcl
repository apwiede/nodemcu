#!/usr/bin/env tclsh8.6

set tokenPatterns [list \
  [list {\A\s+} WHITESPACE] \
  [list {\A\d+} NUMBER] \
  [list {\A0x[[:xdigit:]]+} HEX_NUMBER] \
  [list {\A[*][/]} COMMENT_END] \
  [list {\A/[*][*]} DOC_COMMENT] \
  [list {\A/[*][!]} DOC_COMMENT] \
  [list {\A[/][*]} COMMENT] \
  [list {\A//} LINE_COMMENT] \
  [list {\A[*]} ASTERIKS] \
  [list {\A[@].} DOC_TOKEN] \
  [list {\A\w+} IDENTIFIER] \
  [list {\A[.]} DOT] \
  [list {.} CHAR] \
  [list {\A[\r\n\f]} EOL] \
]

set newlinePattern {\A[\r\n\f\]}

set ::inText false
set ::inComment false
set ::inDocComment false
set ::oneLineComment false
set ::currLine ""
set ::commentType ""

# ================================= scanToken =============================

proc scanToken {line offset typeVar tokenVar} {
  upvar $typeVar tokenType
  upvar $tokenVar token

  set token ""
  set tokenType ""
  set lgth [string length $line]
#puts stderr "parseLine:$line!offset: $offset!inComment: $::inComment!oneLineComment: $::oneLineComment!"
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
      if {[regexp -start $offset $reg $line token]} {
#puts stderr "type: $type!offset: $offset!lgth: $lgth!token: $token![string range $line $offset [expr {$offset + 1}]]!"
        set tokenFound true
        switch $type {
          COMMENT {
#puts stderr "CSTART"
            if {!$::inComment} {
              set ::inDocComment false
              set ::inComment true
              set ::oneLineComment false
              set ::inText true
              set ::txt ""
              set ::commentType $type
            }
#puts stderr "COMM: $::commentType!"
          }
          DOC_COMMENT {
            if {!$::inDocComment} {
              set ::inDocComment true
              set ::oneLineComment false
              set ::inText false
              set ::txt ""
              set ::commentType $type
              set relOffset [expr {$offset + [string length $token]}]
              if {!($relOffset >= $lgth)} {
                if {[string range $line $relOffset $relOffset] ne " "} {
                  set ::inDocComment false
                  set ::inComment true
                  set ::oneLineComment false
                  set ::inComment true
                  set ::inText true
                  set ::txt ""
                  # it is not a DOC_COMMENT!!
                  set ::commentType COMMENT
                  set type COMMENT
                }
              }
            }
#puts stderr "COMM: $::commentType!"
          }
          LINE_COMMENT {
#puts stderr "start LINE_COMMENT: $line!offset: $offset!"
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
          set ::inDocComment false
          set ::onLineComment false
#puts stderr "::txt1: $::txt!"
          set ::txt ""
          set ::inText false
#puts stderr "ret2 offset: $offset!lgth: $lgth!"
          return $offset
        }
        if {!$::inText} {
#puts stderr "offset2: $offset![string range $line $offset end]!"
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
#puts stderr "no token found: $offset!$lgth![string length $line]!"
      }
    }
  }
}

set fd stdin
set isEof false

# scan modes:
#   SCAN_CODE
#   SCAN_COMMENT
#   SCAN_DOC_COMMENT
#   SCAN_LINE_COMMENT
#   SCAN_STRING

set scanMode SCAN_CODE

# object types
#   OBJ_T_UNKNOWN
#   OBJ_T_FILE
#   OBJ_T_VARIABLE
#   OBJ_T_FUNCTION
#   OBJ_T_CLASS
#   OBJ_T_INTERFACE
#   OBJ_T_MEMBERVAR
#   OBJ_T_MEMBERSVAR
#   OBJ_T_MEMBERFUNC
#   OBJ_T_CONSTRUCTOR

set ObjectType OBJ_TY_UNKNOWN

# ================================= nextLine =============================

proc nextLine {} {
  gets $::fd line
  if {[eof $::fd]} {
    set ::isEof true
puts stderr "eof!"
    return ""
  }
#puts stderr "LINE: $line!"
  return $line
}

# ================================= nextToken =============================

# get next Token according to the tokenPatterns array. 
# Reads next line, if current line is completed.

proc nextToken {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  while {true} {
    set offset [scanToken $::currLine $offset tokenType token]
#puts stderr "token: $tokenType: $token!"
    if {($tokenType eq "EOL") || ($tokenType eq "COMMENT_EOL")} {
      set ::currLine [nextLine]
      set lgth [string length $::currLine]
      set offset 0
    }
    if {($offset < 0) || ($::isEof)} {
puts stderr "Eof $offset!$::isEof!"
      break
    }
#puts stderr "nextToken $tokenType!$token!"
    switch $tokenType {
      COMMENT -
      LINE_COMMENT {
        continue
      }
      DOC_COMMENT {
        set ::scanMode SCAN_DOC_COMMENT
      }
      EOL {
        if {$::scanMode eq "SCAN_STRING"} {
          error "Unterminated string literal"
        }
      }
      CHAR {
        if {($token eq "'") || ($token eq "\"")} {
          if {$::scanMode eq SCAN_STRING} {
            if {$::stringType eq $token} {
              set ::stringType ""
              set ::scanMode SCAN_CODE
            }
          } else {
            set ::scanMode SCAN_STRING
            set ::stringType $token
          }
        }
      }
      default {
      }
    }
    return $offset
  }
}

# ================================= nextNoneWsToken =============================

# get next Token according to the tokenPatterns array,
# which is not white-space.
# Reads next line, if current line is completed.

proc nextNoneWsToken {offset tokenType tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  set tokenlst [list]
  set tokenType ""
  set offset [nextToken $offset tokenType token]
  while {$tokenType ne "WHITESPACE"} {
    set offset [nextToken $offset tokenType myToken]
    lappend tokenLst $myToken
  }
  set token $tokenLst
  return $offset
}

# ================================= parseString =============================

proc parseString {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  set token ""
  set tokenType ""
  set offset [nextToken $offset tokenType token]
  while {$::scanMode eq "SCAN_STRING"} {
    set offset [nextToken $offset tokenType myToken]
    append token $myToken
  }
  return $offset
}

# ================================= parseDocComment =============================

proc parseDocComment {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "parseDocComment0!"
  set tokenType ""
  set docTokenLst ""
  set offset [nextToken $offset tokenType token]
  set docToken ""
  set docTokenStr ""
  set inDocToken false
  set docTokenStart false
  while {$tokenType ne "COMMENT_END"} {
    switch $tokenType {
      DOC_TOKEN {
        set docToken $token
        set inDocToken false
        set docTokenStart true
      }
      IDENTIFIER {
        if {$docTokenStart} {
          append docToken $token
          set docTokenStart false
          set inDocToken true
        } else {
          if {$inDocToken} {
            append docTokenStr $token
          }
        }
      }
      EOL {
        if {$inDocToken} {
          set inDocToken false
          lappend docTokenLst [list $docToken $docTokenStr]
          set docToken ""
          set docTokenStr ""
        }
      }
      default {
        if {$inDocToken} {
          append docTokenStr $token
        }
      }
    }
    set offset [nextToken $offset tokenType token]
  }
  set ::scanMode SCAN_CODE
puts stderr "===docTokenLst: $docTokenLst!"
  set result $docTokenLst
  return $offset
}

# ================================= parseCode =============================

proc parseCode {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  set myToken ""
  set tokenType ""
  set offset [nextToken $offset tokenType myToken]
  set codeLst [list]
  while {true} {
    set tokenStr $token 
puts stderr "parseCode: $tokenType $myToken!"
    switch $tokenType {
      COMMENT -
      LINE_COMMENT -
      DOC_COMMENT {
        break
      }
      IDENTIFIER {
      }
      WHITE_SPACE {
      }
      EOL {
      }
      CHAR {
      }
      NUMBER {
      }
      HEX_NUMBER {
      }
      ASTERIKS {
      }
      DOT {
      }
      default {
        set tokenStr ""
      }
    }
    if {$tokenStr ne ""} {
      lappend codeLst $tokenStr
    }
    set offset [nextToken $offset tokenType myToken]
  }
puts stderr "codeLst: $codeLst!"
  return $offset
}

# =============================== parseVariable =============================

proc parseVariable {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "parseVariable"

  return $offset
}

# =============================== parseThis ===================================

proc parseThis {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "parseThis"

  return $offset
}

# =============================== parseFunction ===================================

proc parseFunction {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "parseFunction"

  return $offset
}

# =============================== parsePrototype ===================================

proc parsePrototype {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "parsePrototype"

  return $offset
}

# =============================== parseInterface ===================================

proc parseInterface {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "parseInterface"

  return $offset
}

# =============================== parse ===================================

proc parse {} {
  set offset 0
  set offset [nextToken $offset tokenType token]
puts stderr "START: $tokenType!$token!scanMode: $::scanMode!"
  while {![eof $::fd]} {
puts stderr "parse: $::scanMode!"
    switch $::scanMode {
      SCAN_DOC_COMMENT {
puts stderr "docComment:!"
        set offest [parseDocComment $offset tokenType token]
puts stderr "docComment: $tokenType!$token!scanMode: $::scanMode!"
      }
      SCAN_CODE {
puts stderr "parse: $::scanMode! tokenType: $tokenType!"
        switch $tokenType {
          CHAR {
puts stderr "scanCode CHAR $token!"
            switch $token {
              "\{" {
                incr ::level 1
              }
              "\}" {
                incr ::level -1
                if {$level < 0} {
                  break ; # error!!
                }
              }
              default {
puts stderr "scanCode: default CHAR: $token!"
              }
            }
          }
          IDENTIFIER {
puts stderr "scanCode IDENTIFIER $token!"
            switch $token {
              var {
                set offset [parseVariable $offset tokenType token]
              }
              this {
                set offset [parseThis $offset tokenType token]
              }
              function {
                set offset [parseFunction $offset tokenType token]
              }
              prototype {
                set offset [parsePrototype $offset tokenType token]
              }
              fulfills {
                set offset [parseInterface $offset tokenType token]
              }
              default {
puts stderr "scanCode: default IDENTIFIER: $token!"
              }
            }
          }
          default {
            set offset [nextToken $offset tokenType token]
puts stderr "scanCode default: nextToken: $tokenType!$token!scanMode: $::scanMode!"
          }
        }
if {0} {
                /^{$/               
                    && do { ++$level; next; };
                /^}$/               
                    && do { --$level; next; };
                /^var$/ && $level == 0
                    && do { parse_variable( $context ); next; };
                /^this\./           
                    && do { parse_this( $context, $token ); next; };
                /^function$/        
                    && do { parse_function( $context ); next; };
                /^.+\.prototype/    
                    && do { parse_prototype( $context, $token ); next; };
                /^.+\.fulfills$/    
                    && do { parse_interface( $context, $token ); next; };
}
      }
      default {
puts stderr "default $::scanMode!"
      }
    }
  }
}

# =============================== main ===================================0000

set ::currLine [nextLine]
set ::level 0
parse
puts stderr "END"
