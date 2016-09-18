#!/usr/bin/env tclsh8.6

set newlinePattern {(?n)\A[\r\n\f]}
set identifier {\A\w+}

set tokenPatterns [list \
  [list {\A[@].}            DOC_TOKEN] \
  [list {\A/[*][*]}         DOC_COMMENT] \
  [list {\A/[*][!]}         DOC_COMMENT] \
  [list {\A[*][/]}          COMMENT_END] \
  [list {\A[/][*]}          COMMENT] \
  [list {\A[/][/]}          LINE_COMMENT] \
  [list {\A\d+}             NUMBER] \
  [list {\A0x[[:xdigit:]]+} HEX_NUMBER] \
  [list $identifier         IDENTIFIER] \
  [list {\A\s+}             WHITESPACE] \
  [list {.}                 CHAR] \
  [list {\A[.]}             DOT] \
  [list {\A[']}             SINGLE_QUOTE] \
  [list {\A[\"]}            DOUBLE_QUOTE] \
  [list $newlinePattern     EOL] \
]

set ::inText false
set ::inComment false
set ::inDocComment false
set ::oneLineComment false
set ::currLine ""
set ::commentType ""
set ::parserStack [list]

set ::isEof false
set ::lineNo 0
set ::lastDoc [list]

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

set objectTypeNames [dict create]
dict set objectTypeNames OBJ_T_UNKNOWN     UNDEF
dict set objectTypeNames OBJ_T_FILE        FILE
dict set objectTypeNames OBJ_T_VARIABLE    VARIABLE
dict set objectTypeNames OBJ_T_FUNCTION    FUNCTION
dict set objectTypeNames OBJ_T_CLASS       CLASS
dict set objectTypeNames OBJ_T_INTERFACE   INTERFACE
dict set objectTypeNames OBJ_T_MEMBERVAR   MEMBER_VARIABLE
dict set objectTypeNames OBJ_T_MEMBERSVAR  STATIC_MEMBER_VARIABLE
dict set objectTypeNames OBJ_T_MEMBERFUNC  MEMBER_FUNCTION
dict set objectTypeNames OBJ_T_CONSTRUCTOR CONSTRUCTOR

# -- pdict
#
# Pretty print a dict similar to parray.
#
# USAGE:
#
#   pdict d [i [p [s]]]
#
# WHERE:
#  d - dict value or reference to be printed
#  i - indent level
#  p - prefix string for one level of indent
#  s - separator string between key and value
#
# EXAMPLE:
# % set d [dict create a {1 i 2 j 3 k} b {x y z} c {i m j {q w e r} k o}]
# a {1 i 2 j 3 k} b {x y z} c {i m j {q w e r} k o}
# % pdict $d
# a ->
#   1 -> 'i'
#   2 -> 'j'
#   3 -> 'k'
# b -> 'x y z'
# c ->
#   i -> 'm'
#   j ->
#     q -> 'w'
#     e -> 'r'
#   k -> 'o'
# % pdict d
# dict d
# a ->
# ...
proc pdict { d {i 0} {p "  "} {s " -> "} } {
    set fRepExist [expr {0 < [llength\
            [info commands tcl::unsupported::representation]]}]
    if { (![string is list $d] || [llength $d] == 1)
            && [uplevel 1 [list info exists $d]] } {
        set dictName $d
        unset d
        upvar 1 $dictName d
        puts "dict $dictName"
    }
    if { ! [string is list $d] || [llength $d] % 2 != 0 } {
        return -code error  "error: pdict - argument is not a dict"
    }
    set prefix [string repeat $p $i]
    set max 0
    foreach key [dict keys $d] {
        if { [string length $key] > $max } {
            set max [string length $key]
        }
    }
    dict for {key val} ${d} {
        puts -nonewline "${prefix}[format "%-${max}s" $key]$s"
        if {    $fRepExist && [string match "value is a dict*"\
                    [tcl::unsupported::representation $val]]
                || ! $fRepExist && [string is list $val]
                    && [llength $val] % 2 == 0 } {
            puts ""
            pdict $val [expr {$i+1}] $p $s
        } else {
            puts "'${val}'"
        }
    }
    return
}

# ================================= nextLine =============================

proc nextLine {} {
  gets $::fd line
  append line "\n"
  incr ::lineNo
  if {[eof $::fd]} {
    set ::isEof true
puts stderr "eof!"
    return ""
  }
  set ::currLine $line
  set ::currLineLgth [string length $line]
  set ::currLineOffset 0
puts stderr "LINE ($::lineNo) $line!"
  return $line
}

# ================================= nextToken =============================

# get next Token according to the tokenPatterns array. 
# Reads next line, if current line is completed.

proc nextToken {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

  while {$::currLineOffset >= $::currLineLgth} {
    nextLine
    if {$::isEof} {
      set tokenType EOF
      return ""
    }
  }
  foreach entry $::tokenPatterns {
    set tokenFound false
    foreach {reg tokenType} $entry break
#puts stderr "reg: $reg!"
    if {[regexp -start $::currLineOffset $reg $::currLine token]} {
#puts stderr "type: $type!lgth: $lgth!token: $token![string range $::currLine $::currLineOffset [expr {$::currLineOffset + 1}]]!"
      set tokenFound true
      break
    }
  }
  if {!$tokenFound} {
    error "no token found: $::currLineOffset lineNo: $::lineNo!"
  }
#puts stderr "nextToken2 $tokenType!$token!"
  if {$::isEof} {
    return ""
  }
  # recognize the mode endings
  switch $::scanMode {
    "SCAN_COMMENT" -
    "SCAN_DOC_COMMENT" {
puts stderr "SC_COMM: $tokenType!"
      if {$tokenType eq "COMMENT_END"} {
        set ::scanMode "SCAN_CODE"
      }
    }
    "SCAN_LINE_COMMENT" {
      if {($tokenType eq "WHITESPACE") && ($token eq "\n")} {
        set ::scanMode "SCAN_CODE"
      }
    }
    "SCAN_STRING" {
      if {($tokenType eq "WHITESPACE") && ($token eq "\n")} {
        error "Unterminated string literal."
      }
      if {$token eq $::stringType} {
        set ::stringType ""
        set ::scanMode "SCAN_CODE"
      }
    }
    default {
      # recognize the mode startings
      switch $tokenType {
        COMMENT {
          set ::scanMode SCAN_COMMENT
        }
        DOC_COMMENT {
          set ::scanMode SCAN_DOC_COMMENT;
        }
        LINE_COMMENT {
          set ::scanMode SCAN_LINE_COMMENT
        }
        SINGLE_QUOTE -
        DOUBLE_QUOTE {
          set ::scanMode SCAN_STRING
          set ::stringType $token
        }
        default {
        }
      }
    }
  }
puts stderr "Scanner ($::lineNo): nextToken:  [format %-12s $tokenType] token: [format %-20s '$token'] scanMode: $::scanMode!"
   incr ::currLineOffset [string length $token]
  return $token
}

# ================================= nextNoneWsToken =============================

# get next Token according to the tokenPatterns array,
# which is not white-space.
# Reads next line, if current line is completed.

proc nextNoneWsToken {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "nextNoneWsToken1:!"
  set tokenType ""
  set token [nextToken tokenType]
  while {$tokenType eq "WHITESPACE"} {
    set token [nextToken tokenType]
  }
puts stderr "nextNoneWsToken2: tokenType: $tokenType!$token!"
  return $token
}

# ================================= nextTokenIsLineEnd =============================

proc nextTokenIsLineEnd {} {
  if {[regexp -start $::currLineOffset $::newlinePattern $::currLine]} {
    return true
  }
  return false
}

# ================================= skipLine =============================

proc skipLine {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "skipLine1"
  set token [nextToken tokenType]
  while {!(($tokenType eq "WHITESPACE") && ($token eq "\n"))} {
    set token [nextToken tokenType]
  }
  while {[nextTokenIsLineEnd]} {
    set token [nextToken tokenType]
  }
puts stderr "skipLine2"
}

# ================================= parseString =============================

proc parseString {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "PARSE_STRING"
  set token $::stringType
  set tokenType ""
puts stderr "parseString2"
  while {$::scanMode eq "SCAN_STRING"} {
    set myToken [nextToken myTokenType]
    append token $myToken
puts stderr "parseString4 myTokenType: $myTokenType myToken: $myToken scanMode: $::scanMode token: $token!"
  }
  return $token
}

# ================================= parseComment =============================

proc parseComment {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

  set tokenType ""
  while {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
    set token [nextToken tokenType]
  }
  return $token
}

# ================================= parseType =============================

proc parseType {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "DEB_PARSER ($::lineNo): parseType"
  set myToken [nextNoneWsToken myTypeVar]
  set str ""
  if {(($myToken eq "SINGLE_QUOTE") || ($myToken eq "DOUBLE_QUOTE"))} {
    while {true} {
      set token [nextToken myTokenType]
      if {$token eq $myToken} {
        if {[regexp -start 0 $::newline_pattern $token]} {
          error "Unterminated string literal."
        }
        break
      }
      append str $token
    }
  } else {
    if {[regexp -start 0 ${::identifier}\$ $myToken]} {
      while {true} {
        set token [nextToken myTokenType]
        if {$myTokenType eq "WHITESPACE"} {
          break
        }
        append str $token
      }
    } else {
      error "Identifier or string literal expected"
    }
  }
debug_msg( $DEB_PARSER, "parse_type2: $_ token: '$token'!");
puts stderr "DEB_PARSER ($::lineNo): parseType END [list $str $token]!"
    return  [list $str $token]
}

# ================================= parseDocComment =============================

proc parseDocComment {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "parseDocComment!"
  set doc [dict create]
  dict set doc text ""
  set masterDoc $doc
  set token "/**"
  set tokenStr ""
  set tokenType DOC_COMMENT 
  while {$::scanMode eq "SCAN_DOC_COMMENT"} {
puts stderr "TOK: $tokenStr $tokenType!"
    if {([string range $token 0 0] eq "@") || ([string range $token 0 0] eq "\\")} {
      set tokenStr "${token}[nextToken myTokenType]"
puts stderr "TOK2: $tokenType $tokenStr!$myTokenType $token!"
      if {![dict exists $doc objType]} {
        if {($token eq "@file") || ($token eq "\\file")} {
          dict set doc objType OBJ_T_FILE
        }
        if {($token eq "@fn") || ($token eq "\\fn")} {
          dict set doc objType OBJ_T_FUNCTION
        }
        if {($token eq "@interface") || ($token eq "\\interface")} {
          dict set doc objType OBJ_T_INTERFACE
        }
        if {($token eq "@class") || ($token eq "\\class")} {
          dict set doc objType OBJ_T_CLASS
        }
        if {($token eq "@var") || ($token eq "\\var")} {
          dict set doc objType OBJ_T_VARIABLE
        }
        if {($token eq "@internals") || ($token eq "\\internals")} {
          dict set doc objType OBJ_T_UNKNOWN
        }
        if {($token eq "@ctor") || ($token eq "\\ctor")} {
          dict set doc text "..."
          dict set doc objType OBJ_T_CONSTRUCTOR
          skipLine myTokenType
          set token [nextToken tokenType]
          continue
        }
      } else {
        if {($token eq "@type") || ($token eq "\\type")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
puts stderr "DEB_PARSER ($::lineNo): CTOR1: not yet implemented!!"
          dict set doc text "...."
          set token [parseType myTokenType]
          dict set doc returnType $myToken
          if {![regexp -start 0 $::newlinePattern $token]} {
            skipLine myTokenType
            set token [nextToken tokenType]
          } 
          continue
        }
        if {($token eq "@treturn") || ($token eq "\\treturn")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
          set comment [string range $token 0 0]
          foreach {rtype token} [parseType myTokenType] break
          dict set doc rtype $rtype
          append comment "return $token"
          set tokenStr $comment
        }
        if {($token eq "@tparam") || ($token eq "\\tparam")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
          set comment [string range $token 0 0]
          foreach {type token} [parseType myTokenType] break
          if {[regexp -start 0 $::newlinePattern $token]} {
            error "Missing identifier."
          }
          set param [nextNoneWsToken myTokenType]
          if {$myTokenType ne "IDENTIFIER"} {
            error "identifier expected."
          }
          dict set doc args $param $type
          append comment "param $param"
          set tokenStr $comment
        }
        if {($token eq "@ctor") || ($token eq "\\ctor")} {
          dict append doc text "*/\n"
          dict set doc ctor text "/**\n"
          dict set doc ctor objType OBJ_T_UNKNOWN
          set masterDoc $doc
          set doc [dict get $doc ctor]
          skipLine tokenType
          set token [nextToken tokenType]
          continue
        }
        if {($token eq "@docgen") || ($token eq "\\docgen")} {
          set token [nextToken tokenType]
          set myLine ""
          while {$::scanMode eq "SCAN_DOC_COMMENT"} {
            if {![regexp -start 0 $::newlinePattern $token]} {
              append myLine $token
            }
            set token [nextToken tokenType]
          }
          append myLine [string range $::currLine $::currLineOffset end]
          set ::currLine $myLine
          set ::currLineOffset 0
          set ::currLineLgth [string length $::currLine]
puts stderr "===docgen line: $::currLine!"
          break
        }
      }
puts stderr "doc text1: [dict get $doc text]!"
      dict append doc text $tokenStr
puts stderr "doc text2: [dict get $doc text]!"
    } else {
      if {![dict exists $doc objType] && ($tokenType eq "IDENTIFIER")} {
        dict set doc objType OBJ_T_UNKNOWN
      }
      dict append doc text $token
puts stderr "doc text3: [dict get $doc text]!"
    }
    set token [nextToken tokenType]
  }
  dict append doc text "*/\n"
  if {[dict exists $masterDoc ctor]} {
    dict set masterDoc ctor $doc
  } else {
    set masterDoc $doc
  }
puts stderr "Document for type [dict get $::objectTypeNames [dict get $doc objType]]"
  set ::lastDoc $masterDoc
puts stderr "masterDoc:!"
pdict $masterDoc
}

# ================================= parseCode =============================

proc parseCode {tokenTypeVar} {
  upvar $tokenTypeVar tokenType

puts stderr "parseCode START! #stack: [llength $::parserStack]!"
  while {true} {
    if {[llength $::parserStack] > 0} {
      set entry [lindex $::parserStack 0]
      set ::parserStack [lrange $::parserStack 1 end]
      foreach {tokenType token} $entry break
puts stderr "parserStack2: [llength $::parserStack]!"
    } else {
      set token [nextNoneWsToken tokenType]
    }
    if {$token eq ""} {
puts stderr "parseCode END"
      break
    }
puts stderr "parseCode1: $tokenType $token!$::scanMode!"
    if {$::scanMode eq "SCAN_DOC_COMMENT"} {
      error "Unexpected documentation comment."
    }
    if {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
      parseComment myTokenType
      continue
    }
    break
  }
puts stderr "parseCode END: $tokenType!$token!"
  return $token
}

# ================================= nextTokenIsIdentifier =============================

proc nextTokenIsIdentifier {} {
puts stderr "RR: [string range $::currLine $currLineOffset end]!"
  if {[regexp -start $currLineOffset $::identifier $::currLine]} {
puts stderr "isIdentifier"
    return true
  }
puts stderr "not isIdentifier"
  return false
}

# ================================= nextParserToken =============================

proc nextParserToken {tokenTypeVar} {
  upvar $tokenTypeVar tokenType
  
  set tokenLst [list]
  while {true} {
puts stderr "nextParserToken1: #stack: [llength $::parserStack]!stack: $::parserStack!"
    if {[llength $::parserStack] > 0} {
      set entry [lindex $::parserStack 0]
      set ::parserStack [lreplace $::parserStack 0 0]
      foreach {tokenType token} $entry break
    } else {
      set token [nextNoneWsToken tokenType]
    }
puts stderr "nextParserToken2: $tokenType!$token!$::scanMode"
    if {$token eq ""} {
      break
    }
    if {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
      parseComment myTokenType]
      continue
    }
    if {$::scanMode eq "SCAN_CODE"} {
puts stderr "next_parser_token3: token: $token scanMode: SCAN_CODE"
puts stderr "next_parser_token3a: [string range $::currLine $::currLineOffset end]!"
      if {[regexp -start 0 ${::identifier}\$ $token]} {
puts stderr "IS IDENTIFIER!"
puts stderr "next_parser_token4: $token"
        lappend tokenLst [list $tokenType $token]
puts stderr "next_parser_token4c: tokenLst: $tokenLst!"
        while {true} {
          set token [parseCode myTokenType]
puts stderr "next_parser_token401: parseCode: $myTokenType!$token!"
          if {$token ne "."} {
            break
          }
          lappend tokenLst [list $myTokenType $token]
puts stderr "next_parser_token402: tokenLst: $tokenLst!"
          set token [parseCode myTokenType]
puts stderr "next_parser_token403: tokentype: $myTokenType token: $token!"
          if {$myTokenType eq "IDENTIFIER"} {
puts stderr "ident: $token!"
            lappend tokenLst [list $myTokenType $token]
puts stderr "next_parser_token404: tokenLst: $tokenLst!"
          } else {
puts stderr "MISSING CODE1!!"
            lappend $::parserStack [list CHAR "."]
            break
          }
        }
puts stderr "next_parser_token4b: tokenLst: $tokenLst!"        
        lappend ::parserStack [list $myTokenType $token]
        set token ""
        foreach entry $tokenLst {
          foreach {tokenType myToken} $entry break
          append token $myToken
puts stderr "TK: $token!$myToken!"
        }
puts stderr "next_parser_token7: token: $token!"
        break
      }
#      break
    } else {
puts stderr "nextParserToken2: $tokenType!$token!$::scanMode"
      if {$::scanMode eq "SCAN_STRING"} {
        set token [parseString myTokenType]
puts stderr "parseString: $myTokenType!$token!"
        break
      } else {
        if {$::scanMode eq "SCAN_DOC_COMMENT"} {
puts stderr "scanDocDocument"
          break
        }
      }
    }
  }
puts stderr "nextParserToken End token: $token"
  return $token
}

# =============================== parseVariable =============================

proc parseVariable {level contextVar tokenTypeVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType

puts stderr "PARSE_VARIABLE"
puts stderr "DEB_DETECTOR ($::lineNo); Find Variable in '[dict get $context name]'."
  set token [parseCode tokenType]

puts stderr "PARSE_VARIABLE1 token: $token"
  if {[dict get $context objType] eq "OBJ_T_FILE"} {
puts stderr "PARSE_VARIABLE2"
    if {$tokenType ne "IDENTIFIER"} {
      error "Variable name expected, found '$token'."
    }
    dict set context objs $token name $token
    dict set context objs $token objType OBJ_T_VARIABLE
    dict set context objs $token scope context
    set varContext [dict get $context objs $token]
puts stderr "DEB_DATABASE ($::lineNo): Variable '$token'"
    if {$::lastDoc ne [list]} {
      if {[dict get $::lastDoc objType] eq "OBJ_T_UNKNOWN"} {
        if {[dict exists $varContext doc]} {
          puts stderr "warning Comment for '$token' already exists, ignoring new."
        } else {
          dict set varContext doc $::lastDoc
puts stderr "DEB_PARSER ($::lineNo): Comment for variable '$token'." 
        }
      }
    }
  }
puts stderr "DEB_DETECTOR ($::lineNo): Found Variable in '[dict get $context name]'."
puts stderr "context: $context!"
  set ::lastDoc undef
puts stderr "PARSE_VARIABLE3"
}

# =============================== parseThis ===================================

proc parseThis {level contextVar tokenTypeVar token} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
puts stderr "PARSE_THIS"
puts stderr "DEB_DETECTOR ($::lineNo): Find member variable in ' [dict get $context name]' with token '$token'." 

  set myToken [lindex [split $token "."] 1]
  if {[regexp -start 0 ${::identifier} $myToken]} {
    set token [parseCode tokenType]
    if {$token ne "="} {
puts stderr "DEB_DETECTOR ($::lineNo): Not found member variable in '[dict get $context name]' with token '$token'." 
      return
    }
        
    if {![dict exists $context members $token]} {
      dict set context members $token name $token
      dict set context members $token objType OBJ_T_MEMBERVAR 
      dict set context members $token scope context 
            
puts stderr "DEB_DATABASE ($::lineNo): Added member variable '$token' to class '[dict get $context name]'." 
    }
    if {$::lastDoc ne [list]} {
      if {[dict exists $context members $token doc]} {
        puts stderr "warning Comment for '$token' already exists, ignoring new."
      } else {
        dict set context members $token doc $::lastDoc
puts stderr "DEB_DATABASE ($::lineNo): Comment for member variable '$token'."
      }
    }
    set ::lastDoc undef
  }

puts stderr "DEB_DETECTOR ($::lineNo): Found Member variable in '[dict get $context name]'."
puts stderr "PARSE_THIS END"
}

# =============================== parseFunction ===================================

proc parseFunction {level contextVar tokenTypeVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
puts stderr "PARSE_FUNCTION"
puts stderr "parseFunction LEVEL: $level Find Function Definition in: [dict get $context name]"
  
  set token [parseCode tokenType]
puts stderr "parseFunction1: $tokenType $token!"
  if {($tokenType ne "CHAR") && ($token ne "(")} {
    if {$tokenType ne "IDENTIFIER"} {
      error "Function name expected, found '$token'."
    }
    set name $token
    set token [parseCode tokenType]
  } else {
    if {![dict exists $context anonymous]} {
      dict set context anonymous 0
    }
    set name "?[dict get $context anonymous]"
    dict set context anonymous [expr {[dict get $context anonymous] + 1}]
  }
  if {![dict exists $context objs $name]} {
    dict set context objs $name name $name
    dict set context objs $name objType OBJ_T_FUNCTION
#    dict set context objs $name scope $context
    set fcnContext [dict get $context objs $name]
  } else {
    set fcnContext [dict get $context objs $name]
    if {([dict get $fcnContext objType] ne "OBJ_T_FUNCTION") && ([dict get $fcnContext objType] ne "OBJ_T_CLASS")} {
      error "Function or class expected, found '[dict get $fcnContext objType]'"
    }
    if {[dict exists $fcnContext unkown]} {
      set fcnContext [dict remove $fcnCntext unknown]
    }
  }
  if {[string range $name 0 0] eq "?"} {
    set myName anonymous
  } else {
    set myName function
  }
puts stderr "DataBase ($::lineNo) added $myName $name lastDoc: $::lastDoc!"
  if {($::lastDoc ne [list]) && ($::lastDoc ne "undef")} {
    if {[dict get $::lastDoc objType] eq "OBJ_T_UNKNOWN"} {
      if {[dict exists $fcnContext doc]} {
        puts stderr "warning Comment for '$name' already exists, ignoring new."
      } else {
        dict set fcnContext doc $::lastDoc
puts stderr "Database ($::lineNo) Comment for function '$name'."
      }
    }
    if {[dict get $::lastDoc objType] eq "OBJ_T_CONSTRUCTOR"} {
      if {[dict exists $fcnContext ctor]} {
        puts stderr "warning Constructor comment for '$name' already exists, ignoring new."
      } else {
        dict set fcnContext ctor $::lastDoc
puts stderr "Database ($::lineNo) Comment for constructor '$name'."
      }
    }
    set last_doc [list]
  }
puts stderr "BEFORE ARGS: tokenType: $tokenType token: $token!"
  if {($tokenType ne "CHAR") && ($token ne "(")} {
    error "'(' expected, found '$token'." 
  }
  dict set fcnContext args [list]
  while {true} {
    set token [parseCode tokenType]
puts stderr "args token: $token!"
    if {($tokenType eq "CHAR") && ($token eq ")")} {
      break
    }
    if {($tokenType eq "CHAR") && ($token eq "," )} {
      continue
    }
    if {$tokenType ne "IDENTIFIER"} {
      error "Function parameter name expected, found '$token'."
    }
     dict lappend fcnContext args [list name $token]
  }
puts stderr ">args done. '[string range $::currLine $::currLineOffset end]!"

  set LC "\{"
  set RC "\}"
  set token [parseCode tokenType]
  if {($tokenType eq "CHAR") && ($token ne $LC)} {
    error "'$LC' expected, found '$token'."
  }
  if {![dict exists $fcnContext objs]} {
    dict set fcnContext objs [list]
  }
  set objs [dict get $fcnContext objs]
  lappend fcnContext symbolStack $objs
  set fcnContext [dict remove $fcnContext objs]
# FIXME!!!
#    $fnContext->{objs}{$_} = $objs->{$_} foreach ( keys %$objs );
  dict set fcnContext objs [list]
  set token [parse fcnContext level tokenType]
  set fcnContext [dict remove $fcnContext objs]
  dict set fcnContext objs  [dict get $fcnContext symbolStack]

  if {[dict exists $context name]} {
puts stderr "Detector ($::lineNo) Found Function Definition in '[dict get $context name]'." 
    set token $name
  } else {
puts stderr "Detector ($::lineNo) Not found Function Definition." 
    set token "UNKNOWN"
  }
}

# =============================== createBase ===================================

proc createBase {level contextVar name} {
  upvar $contextVar context
  set base $name
  set scope $context
  while {($scope ne "") && ![dict exists $scope objs $base]} {
    set context $scope
    set scope [dict get $scope scope]
  }
  if {$scope eq ""} {
    dict set context objs $base name $base
    dict set context objs $base objType OBJ_T_CLASS
    dict set context objs $base scope $context
    dict set context objs $base unknown 1
    set scope $context;
    puts stderr "DEB_DATABASE Added missing base class '$base'." 
  }
  return $scope
}
  
# =============================== parsePrototype ===================================

proc parsePrototype {level contextVar tokenTypeVar token} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
puts stderr "PARSE_PROTOTYPE"

puts stderr "DEB_DETECTOR Find Prototype Definition in '[dict get $context name]' with token '$token'."
  set myToken [parseCode tokenType]
  if {$myToken ne "="} {
    puts stderr "DEB_DETECTOR Not found Prototype Definition in '[dict get $context name]' with token '$token'."
    return;
  }

  set lst [split $token "."]
  set name [lindex $lst 0]
  if {![regexp -start 0 "${::identifier}\$" $name]} {
    error "Not a valid identifier '$name' for prototype definition."
  }
    
  if {![dict exists $context objs $name] && [dict exists $context members $name]} {
    if {[dict get $context members $name otype] ne "OBJ_T_MEMBERFUNC"} {
      error "Wrong prototype assignment to '$name' of type [dict get $::objectTypeNames [dict get $context members $name otype]]."
    }
    dict set context objs $name [dict get $context members $name]
    dict set $context members [dict remove $context members $name]]
    dict set context objs $name objType OBJ_T_CLASS
    puts stderr "DEB_DATABASE '$name' is a nested class."
  }
  if {[dict exists $context objs $name]} {
    set fcnContext [dict get $context objs $name]
  } else {
    set fcnContext [createBase $level context $name]
    set fcnContext [dict get $fnContext objs $name]
  }
  if {[dict get $fcnContext objType eq "OBJ_T_FUNCTION"} {
    dict set fcnContext otype OBJ_T_CLASS
  }
  if {([dict get $fcnContext objType] ne "OBJ_T_CLASS") && ([dict get $fcnContext objType] ne "OBJ_T_INTERFACEC")} {
    error "Prototype assignment to invalid type '[dict get $::objectTypeNames [dict get $fcnContext objType]].'."
  }
puts stderr "DEB_DATABASE '$name' is a class." 
 
  set midPart [lindex $lst 1]

  if {$midPart eq "prototype"} {
    lappend ::parserStack [list CHAR "="]
    parsePrototype $level fcnContext tokenType $token
    puts stderr "DEB_DETECTOR Found Nested Prototype Definition in '[dict get $context name]'."
    return
  }
  if {$midPart eq "fulfills"} {
    lappend ::parserStack [list CHAR "="]
    parseInterface $level fcnContext tokenType $token
    puts stderr "DEB_DETECTOR Found Nested Interface Definition in '[dict get $context name]'."
    return
  }
  if {($name ne "") && ($token ne "")} {
    puts stderr "warning Unknown code construction '$token' in prototype definition of '$name'." 
    while {true} {
      set myToken [parseCode tokenType]
      if {$myToken eq ";" } {
        break
      }
    }
    set ::lastDoc undef
    puts stderr "DEB_DETECTOR Not found Prototype Definition in '[dict get $context name]'."
    return
  }
    
  set member $token
  if {$member eq ""} {
    set token [parseCode tokenType]
    if {$token ne "new"} {
      error "'new' expected, found '$token'."
    }
    set token [parseCode tokenType]
    if {![regexp -start 0 "${::identifier}\$" $token]} {
      error "Identifier expected, found '$token'." 
    }
    set base $name
    while {true} {
      set token [parseCode tokenType]
      if {[regexp -start 0 {[()]} $token]} {
      }
      if {$token ne ";"} {
        error "';' expected, found '$token'."
      } 
      set scope [create_base $context $base]
      if {[dict get $scope objs $base objType] eq "OBJ_T_FUNCTION"} {
        dict set scope objs $base objType "OBJ_T_CLASS"
      }
      if {[dict get $scope objs $base objTtype] ne "OBJ_T_CLASS"} {
        error "'$base' is not of type class, but of type '[dict get $::objectTypeNames [dict get $scope objs $base objType]]'." 
      }
      dict set fcnContext base [dict get $scope objs $base]
      puts stderr "DEB_DATABASE Set '$base' as base for class '$name'." 
      if {$::lastDoc ne [list]} {
        if {[dict exists $fcnContext doc]} {
          puts stderr "warning Comment for '$name' already exists, ignoring new."
        } else {
          dict set fcnContext doc $::lastDoc
          puts stderr "DEB_DATABASE Comment for class '$name'."
        }
      }
      set ::lastDoc undef
    }
  } else {
    set doc $::lastDoc;
    set ::lastDoc undef
        
    set token [parseCode tokenType]
    if {[regexp -start 0 "$::identifier\$" $token]} {
      set base $token
      set end 1
      if {$base eq "function"} {
        set base [parseFunction level context tokentype]
        set end 0
      } else {
        set myToken [parseCode tokenType]
        while {$myToken ne ""} {
          switch -regexp $myToken {
            {;} {
              set end 0
              break
            }
            {.} {
              $myToken =~ /$identifier$/ 
              append token $myToken
              continue
            }
            prototype {
              $myToken !~ /\.prototype\.$/ 
              && $token =~ /\.$/ 
              myToken $token
              continue
            }
            default {
              if {[regexp -start 0 $::identifier $myToken]} {
                $myToken =~ /\.prototype\.$/ 
                && !/^prototype$/
                append myToken $token
                continue
              }
            }
          }
          set base "???"
          #syntax_err( "Unexpected tokens '$token' and '$_'"
          #   ." in prototype assignment" );
        }
      }
      set scope $context
      while {$scope && ![dict exists $scope objs $base]} {
          set scope [dict get $scope scope]
      }
      if {$scope ne ""} {
        if {$end} {
          set token [parseCode tokenType]
          if {$token ne ";"} {
            createStaticVar $fcnContext $member
          }
        }
      } else {
        set scope [dict get $scope- objs $base]
        if {[regexp -start 0 "[.]" $token]} {
          $token =~ s/^$base\.prototype\.(.*)$/\1/;
          set scope [findMember $scope $token]
        } else {
          if {[dict get $scope objType] eq "OBJ_T_FUNCTION"} {
            dict set scope objType OBJ_T_MEMBERFUNC
          }
          if {[dict get $scope objType] eq "OBJ_T_MEMBERVAR"} {
            dict set scope objType OBJ_T_MEMBERSVAR
          }
          if {([dict get $scope objType] ne "OBJ_T_MEMBERFUNC")
                  && ([dict get $scope objType] ne "OBJ_T_MEMBERSVAR")
                  && ([dict get $fcnContext base] ne $scope)} {
            error "$base is a '[ dict get $::objectTypeNames [dict get $scope objType]]' and not a member."
          }
        }
        if {[dict get $fcnContext base] ne $scope} {
          set type ""
          if {[dict get $scope objType] eq "OBJ_T_MEMBERSVAR"} {
            append type "static "
          }
          append type member
          set what ""
          if {[dict get $scope objType] eq "OBJ_T_MEMBERFUNC"} {
            append what "function"
          } else {
            append what "variable"
          }
          append what " with global name '"
          puts stderr "DEB_DATABASE '$member' is a ${type} $what [dict get $scope name]"
            dict set fcnContext members $member $scope
        }
        if {$end} {
          set token [parseCode tokenType]
          if {$token ne ";" } {
            error "';' expected, found '$token'."
          }
        }
      }
    } else {
      createStaticVar $fcnContext $member
      while {true} {
        set token [parseCode tokenType]
        if {$token eq ";"} {
          break
        }
      }
    }
      
    if {$doc  ne "" && [dict exists $fcnContext members $member]} {
      if {[dict exists $fcnContext members $member doc]} {
        puts stderr "warning( Comment for '$member' already exists, ignoring new."
      }
    } else {
      dict set fcnContext members $member doc $doc
      puts stderr "DEB_DATABASE Comment for member '$member'."
    }
  }
puts stderr "DEB_DETECTOR Found Prototype Definition in '[dict get $context name]'."
}

# =============================== parseInterface ===================================

proc parseInterface {level contextVar tokenTypeVar token} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
puts stderr "PARSE_INTERFACE"

}

# =============================== parse ===================================

proc parse {contextVar levelVar tokenType} {
  upvar $contextVar context
  upvar $levelVar level

  set token ""
puts stderr ">parse: LEVEL: $level token: $token contextName: [dict get $context name]!"
puts stderr "Parser1 ($::lineNo) token: $token!"
puts stderr ">>>scanMode1:!"
#puts stderr "START: $tokenType!$token!scanMode: $::scanMode!"
  while {true} {
puts stderr "Parser BEGIN LOOP ($::lineNo)"
    set token [nextParserToken tokenType]
puts stderr "Parser3 ($::lineNo) tokenType: $tokenType token: $token scanMode: $::scanMode!"
    if {$token eq ""} {
      break
    }
    switch $::scanMode {
      SCAN_CODE {
puts stderr "parse: >>>scanMode2: $::scanMode! tokenType: $tokenType!"
        switch $tokenType {
          CHAR {
puts stderr "scanCode CHAR $token!"
            switch $token {
              "\{" {
                incr ::level 1
                continue
              }
              "\}" {
                incr ::level -1
                if {$level < 0} {
                  break
                }
              }
              default {
puts stderr "scanCode: default CHAR: $token!"
              }
            }
          }
          IDENTIFIER {
puts stderr "Parser ($::lineNo) scanCode IDENTIFIER $token!"
            switch -regexp -- $token {
              var {
puts stderr ">>> variable"
                if {$level == 0} {
                  parseVariable $level context tokenType
                }
              }
              {^this[.]} {
puts stderr ">>> this"
                parseThis $level context tokenType $token
                continue
              }
              function {
puts stderr ">>> function"
                parseFunction $level context tokenType
                continue
              }
              {^.+[.]prototype} {
                parsePrototype $level context tokenType $token
                continue
              }
              {^.+[,]fulfills} {
                parseInterface $level context tokenType $token
                continue
              }
              default {
puts stderr "scanCode: IDENTIFIER default: $token!"
              }
            }
          }
          default {
            set token [nextToken tokenType]
puts stderr "scanCode default: nextToken: $tokenType!$token!scanMode: $::scanMode!"
          }
        }
      }
      SCAN_DOC_COMMENT {
puts stderr "docComment:!"
        set token [parseDocComment tokenType]
puts stderr "docComment: $tokenType!$token!scanMode: $::scanMode!"
      }
      default {
puts stderr "default $::scanMode!"
      }
    }
puts stderr "END LOOP"
  }
}

# =============================== main ===================================0000

if {[llength $argv] == 0} {
  puts stderr "usage: $::argv0 <file name>"
  exit 1
}
set level 0
set context [dict create]
dict set context name [lindex $argv 0]
dict set context objType OBJ_T_FILE
dict set context scope undef
set ::fd [open [lindex $argv 0] r]

set file $context
set ::curLine ""
set ::currLineOffset 0
set ::currLineLgth 0

set token  [parse context level tokenType]
puts stderr "context:"
pdict $context
#puts stderr "masterDoc"
#pdict $masterDoc
puts stderr "lastDoc"
pdict $::lastDoc
puts stderr "END"
