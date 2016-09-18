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
puts stderr "LINE ($::lineNo) $line!"
  return $line
}

# ================================= nextToken =============================

# get next Token according to the tokenPatterns array. 
# Reads next line, if current line is completed.

proc nextToken {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  set lgth [string length $::currLine]
  while {$offset >= $lgth} {
    nextLine
    set lgth [string length $::currLine]
    set offset 0
    if {$::isEof} {
      set tokenType EOF
      set token ""
      return $offset
    }
  }
  foreach entry $::tokenPatterns {
    set tokenFound false
    foreach {reg tokenType} $entry break
#puts stderr "reg: $reg!"
#puts stderr "offset: $offset!type: $tokenType!"
    if {[regexp -start $offset $reg $::currLine token]} {
#puts stderr "type: $type!offset: $offset!lgth: $lgth!token: $token![string range $::currLine $offset [expr {$offset + 1}]]!"
      set tokenFound true
      break
    }
  }
  if {!$tokenFound} {
    error "no token found: $offset lineNo: $::lineNo!"
  }
#puts stderr "nextToken2 $tokenType!$token!"
  if {$::isEof} {
    return -1
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
      if {$tokenType eq "EOL"} {
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
          set::scanMode SCAN_COMMENT
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
puts stderr "Scanner ($::lineNo): nextToken:  [format %-12s $tokenType] token: [format %-20s '$token'] offset: [format %-5d $offset] scanMode: $::scanMode!"
  return [expr {$offset + [string length $token]}]
}

# ================================= nextNoneWsToken =============================

# get next Token according to the tokenPatterns array,
# which is not white-space.
# Reads next line, if current line is completed.

proc nextNoneWsToken {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "nextNoneWsToken1:!"
  set tokenType ""
  set offset [nextToken $offset tokenType token]
  if {$::isEof} {
    return -1
  }
  while {$tokenType eq "WHITESPACE"} {
    set offset [nextToken $offset tokenType token]
    if {$::isEof} {
      return -1
    }
  }
puts stderr "nextNoneWsToken2: tokenType: $tokenType!$token!"
#puts stderr "nnwt: $tokenType!$token!$::lineNo!"
  return $offset
}

# ================================= nextTokenIsLineEnd =============================

proc nextTokenIsLineEnd {offset} {
  if {[regexp -start $offset $::newlinePattern $::currLine]} {
    return true
  }
  return false
}

# ================================= skipLine =============================

proc skipLine {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "skipLine1"
  set offset [nextToken $offset tokenType token]
  if {$::isEof} {
    return -1
  }
  while {!(($tokenType eq "WHITESPACE") && ($token eq "\n"))} {
    set offset [nextToken $offset tokenType token]
    if {$::isEof} {
      return -1
    }
  }
  while {[nextTokenIsLineEnd $offset]} {
    set offset [nextToken $offset tokenType token]
    if {$::isEof} {
      return -1
    }
  }
puts stderr "skipLine2"
  return $offset
}

# ================================= parseString =============================

proc parseString {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "PARSE_STRING"
  set token $::stringType
  set tokenType ""
puts stderr "parseString2"
#  set offset [nextToken $offset myTokenType mytoken]
#puts stderr "parseString3 myTokenType: $myTokenType myToken: myToken scanMode: $::scanMode"
  while {$::scanMode eq "SCAN_STRING"} {
    set offset [nextToken $offset myTokenType myToken]
    if {$::isEof} {
      return -1
    }
    append token $myToken
puts stderr "parseString4 myTokenType: $myTokenType myToken: myToken scanMode: $::scanMode token: $token!"
  }
  return $offset
}

# ================================= parseComment =============================

proc parseComment {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

  set tokenType ""
  set offset [nextToken $offset mytokenType mytoken]
  if {$::isEof} {
    return -1
  }
  while {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
    set offset [nextToken $offset tokenType myToken]
    if {$::isEof} {
      return -1
    }
    append token $myToken
  }
  return $offset
}

# ================================= parseDocComment =============================

proc parseDocComment {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "parseDocComment!"
  set doc [dict create]
  dict set doc text ""
  set masterDoc $doc
  set token "/**"
  set tokenType DOC_COMMENT 
#  set offset [nextToken $offset tokenType token]
#  if {$::isEof} {
#    return -1
#  }
  while {$::scanMode eq "SCAN_DOC_COMMENT"} {
puts stderr "TOK: $token $tokenType!"
    if {([string range $token 0 0] eq "@") || ([string range $token 0 0] eq "\\")} {
      set offset [nextToken $offset myTokenType myToken]
      if {$::isEof} {
        return -1
      }
      append token $myToken
puts stderr "TOK2: $tokenType $token!$myTokenType $myToken!"
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
          set offset [skipLine $offset myTokenType myToken]
          set offset [nextToken $offset tokenType token]
          if {$::isEof} {
            return -1
          }
          continue
        }
      } else {
        if {($token eq "@type") || ($token eq "\\type")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
          dict set doc text "...."
          set offset [parseType $offset myTokenType myToken]
          dict set doc returnType $myToken
          if {$myToken ne "EOL"} {
            set offset [skipLine $offset myTokenType myToken]
            set offset [nextToken $offset tokenType token]
          } 
        }
        if {($token eq "@treturn") || ($token eq "\\treturn")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
          set comment "YYY"
          set offset [parseType $offset myTokenType myToken]
          dict set doc returnType $myToken
          append comment "return $myToken"
        }
        if {($token eq "@type") || ($token eq "\\type")} {
          if {[dict exists $doc returnType]} {
            puts stderr "warning Return type already defined!"
          }
          set comment "YYY"
          set offset [parseType $offset myTokenType myType]
          if {$myToken eq "\n"} {  ; # FIXME! new_line_pattern!
            error "Missing identifier."
          }
          set offset [nextNoneWsToken $offset myTokenType myToken]
          if {$myTokenType ne "IDENTIFIER"} {
            error "identifier expected."
          }
          dict set doc args $myToken $myType
        }
        if {($token eq "@ctor") || ($token eq "\\ctor")} {
          dict append doc text "*/\n"
          dict set doc ctor text "/**\n"
          dict set doc ctor objType OBJ_T_UNKNOWN
          set masterDoc $doc
          set doc [dict get $doc ctor]
          set offset [skipLine $offset tokenType token]
          set offset [nextToken $offset tokenType token]
          continue
        }
        if {($token eq "@docgen") || ($token eq "\\docgen")} {
          set offset [nextToken $offset tokenType token]
          while {$::scanMode eq "SCAN_DOC_COMMENT"} {
            if {token eq "\n"} {  ; # FIXME! new_line_pattern!
              set offset [nextToken $offset tokenType token]
            }
          }
          break
        }
      }
      dict append doc text $token
    } else {
      if {![dict exists $doc objType] && ($tokenType eq "IDENTIFIER")} {
        dict set doc objType OBJ_T_UNKNOWN
      }
      dict append doc text $token
puts stderr "text3: [dict get $doc text]!"
    }
    set offset [nextToken $offset tokenType token]
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
  return $offset
}

# ================================= parseCode =============================

proc parseCode {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "parseCode0!offset: $offset"
    if {[llength $::parserStack] > 0} {
      set entry [lindex $::parserStack 0]
      set ::parserStack [lrange $::parserStack 1 end]
      foreach {tokenType token} $entry break
puts stderr "parserStack2: [llength $::parserStack]!"
    } else {
      set offset [nextNoneWsToken $offset tokenType token]
    }
puts stderr "parseCode1: $tokenType $token!$::scanMode!"
    if {$::scanMode eq "SCAN_DOC_COMMENT"} {
      error "Unexpected documentation comment."
    }
    if {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
      set offset [parseComment $offset myTokenType myToken]
    }
puts stderr "parseCode: $tokenType!$token!offset: $offset"
  return $offset
}

# ================================= nextTokenIsIdentifier =============================

proc nextTokenIsIdentifier {offset} {
puts stderr "RR: [string range $::currLine $offset end]!"
  if {[regexp -start $offset $::identifier $::currLine]} {
puts stderr "isIdentifier"
    return true
  }
puts stderr "not isIdentifier"
  return false
}

# ================================= nextParserToken =============================

proc nextParserToken {offset tokenTypeVar tokenVar} {
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
  
  set tokenLst [list]
  while {true} {
puts stderr "nextParserToken1: #stack: [llength $::parserStack]!stack: $::parserStack!"
    if {[llength $::parserStack] > 0} {
      set entry [lindex $::parserStack 0]
      set ::parserStack [lreplace $::parserStack 0 0]
      foreach {tokenType token} $entry break
    } else {
      set offset [nextNoneWsToken $offset tokenType token]
      if {$::isEof} {
        return -1
      }
    }
puts stderr "nextParserToken2: $tokenType!$token!$::scanMode"
    if {($::scanMode eq "SCAN_COMMENT") || ($::scanMode eq "SCAN_LINE_COMMENT")} {
      set offset [parseComment $offset myTokenType myToken]
      continue
    }
    if {$::scanMode eq "SCAN_CODE"} {
puts stderr "next_parser_token3: token: $token scanMode: SCAN_CODE"
puts stderr "next_parser_token3a: [string range $::currLine $offset end]!"
      if {[regexp -start 0 ${::identifier}\$ $token]} {
puts stderr "IS IDENTIFIER!"
puts stderr "next_parser_token4: $token"
        lappend tokenLst [list $tokenType $token]
puts stderr "next_parser_token4c: tokenLst: $tokenLst!"
        while {true} {
          set offset [parseCode $offset myTokenType myToken]
puts stderr "next_parser_token401: parseCode: $myTokenType!$myToken!"
          if {$myTokenType ne "DOT"} {
            break
          }
          lappend tokenLst [list $myTokenType $myToken]
puts stderr "next_parser_token402: tokenLst: $tokenLst!"
          set offset [parseCode $offset myTokenType myToken]
puts stderr "next_parser_token403: tokentype: $myTokenType token: $myToken!"
          if {$myTokenType eq "IDENTIFIER"} {
puts stderr "ident: $myToken!"
            lappend tokenLst [list $myTokenType $myToken]
puts stderr "next_parser_token404: tokenLst: $tokenLst!"
          } else {
puts stderr "MISSING CODE1!!"
          }
        }
puts stderr "tokenLst: $tokenLst!"        
        lappend ::parserStack [list $myTokenType $myToken]
        foreach {tokenType token} [lindex $tokenLst 0] break
      }
      break
    } else {
puts stderr "nextParserToken2: $tokenType!$token!$::scanMode"
      if {$::scanMode eq "SCAN_STRING"} {
        set offset [parseString $offset myTokenType myToken]
puts stderr "parseString: $myTokenType!$myToken!"
        break
      } else {
        if {$::scanMode eq "SCAN_DOC_COMMENT"} {
puts stderr "scanDocDocument"
          break
        }
      }
    }
  }
puts stderr "nextParserToken End offset: $offset"
  return $offset
}

# =============================== parseVariable =============================

proc parseVariable {offset level contextVar tokenTypeVar tokenVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token

puts stderr "PARSE_VARIABLE"
puts stderr "DEB_DETECTOR ($::lineNo); Find Variable in '[dict get $context name]'."
  set offset [parseCode $offset tokenType token]

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
  set tokenType "CHAR"
  set token ";"
  return $offset
}

# =============================== parseThis ===================================

proc parseThis {offset level contextVar tokenTypeVar tokenVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "PARSE_THIS"
puts stderr "DEB_DETECTOR ($::lineNo): Find member variable in ' [dict get $context name]' with token '$token'." 

  if {$token =~ s/^this\.($identifier)$/\1/ } {
    set offset [parseCode $offset tokenType, token]
    if {$token ne "="} {
puts stderr "DEB_DETECTOR ($::lineNo): Not found member variable in '[dict get $context name]' with token '$token'." 
      return
    }
        
    if {![dict exists $context members $token]} {
      dict set context members $token name $token
      dict set context members $token name objType OBJ_T_MEMBERVAR 
      dict set context members $token name scope context 
            
puts stderr "DEB_DATABASE ($::lineNo): Added member variable '$token' to class '[dict get $context name]'." 
    }
    if {[$::lastdoc me [list]} {
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
  return $offset
}

# =============================== parseFunction ===================================

proc parseFunction {offset level contextVar tokenTypeVar tokenVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "PARSE_FUNCTION"
puts stderr "parseFunction LEVEL: $level Find Function Definition in: [dict get $context name]"
  
  set offset [parseCode $offset tokenType token]
puts stderr "parseFunction1: $tokenType $token!"
  if {$::isEof} {
    return -1
  }
  if {($tokenType ne "CHAR") && ($token ne "(")} {
    if {$tokenType ne "IDENTIFIER"} {
      error "Function name expected, found '$token'."
    }
    set name $token
    set offset [parseCode $offset tokenType token]
    if {$::isEof} {
      return -1
    }
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
puts stderr "DataBase ($::lineNo) added $myName $name"
  if {$::lastDoc ne [list]} {
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
  if {($tokenType ne "CHAR") && ($token ne "(")} {
    error "'(' expected, found '$token'." 
    dict set fcnContext args [list]
    while {true} {
      set offset [parse_code $offset tokenType token]
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
  }

  set LC "\{"
  set RC "\}"
  set offset [parseCode $offset tokenType token]
  if {$::isEof} {
    return -1
  }
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
  parse fcnContext $offset level $tokenType $token
  set fcnContext [dict remove $fcnContext objs]
  dict set fcnContext objs  [dict get $fcnContext symbolStack]

if {[dict exists $context name]} {
puts stderr "Detector ($::lineNo) Found Function Definition in '[dict get $context name]'." 
  set token $name
} else {
puts stderr "Detector ($::lineNo) Not found Function Definition." 
  set token "UNKNOWN"
}

  return $offset
}

# =============================== parsePrototype ===================================

proc parsePrototype {offset level contextVar tokenTypeVar tokenVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "PARSE_PROTOTYPE"

  return $offset
}

# =============================== parseInterface ===================================

proc parseInterface {offset level contextVar tokenTypeVar tokenVar} {
  upvar $contextVar context
  upvar $tokenTypeVar tokenType
  upvar $tokenVar token
puts stderr "PARSE_INTERFACE"

  return $offset
}

# =============================== parse ===================================

proc parse {contextVar offset levelVar tokenType token} {
  upvar $contextVar context
  upvar $levelVar level

puts stderr "LEVEL: $level contextName: [dict get $context name]!"
puts stderr "Parser1 ($::lineNo) token: $token!"
puts stderr ">>>scanMode1:!"
#  set offset [nextToken $offset tokenType token]
#puts stderr "START: $tokenType!$token!scanMode: $::scanMode!"
  while {![eof $::fd]} {
puts stderr "Parser BEGIN LOOP ($::lineNo)"
    set offset [nextParserToken $offset tokenType token]
    if {$::isEof} {
      return -1
    }
puts stderr "Parser3 ($::lineNo) tokenType: $tokenType token: $token scanMode: $::scanMode!offset: $offset"
    switch $::scanMode {
      SCAN_DOC_COMMENT {
puts stderr "docComment:!"
        set offest [parseDocComment $offset tokenType token]
        if {$::isEof} {
          return -1
        }
puts stderr "docComment: $tokenType!$token!scanMode: $::scanMode!"
      }
      SCAN_CODE {
puts stderr "parse: >>>scanMode2: $::scanMode! tokenType: $tokenType!"
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
            switch $token {
              var {
                set offset [parseVariable $offset $level context tokenType token]
puts stderr ">>> variable"
              }
              this {
                set offset [parseThis $offset $level context tokenType token]
              }
              function {
                set offset [parseFunction $offset $level context tokenType token]
              }
              prototype {
                set offset [parsePrototype $offset $level context tokenType token]
              }
              fulfills {
                set offset [parseInterface $offset $level context tokenType token]
              }
              default {
puts stderr "scanCode: default IDENTIFIER: $token!"
              }
            }
            if {$::isEof} {
              return -1
            }
          }
          default {
            set offset [nextToken $offset tokenType token]
            if {$::isEof} {
              return -1
            }
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
parse context 0 level "" ""
puts stderr "END"
