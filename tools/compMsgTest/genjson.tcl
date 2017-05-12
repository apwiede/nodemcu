# -- genjson
#
# produce as output a json object
#
# USAGE:
#
#   genjson d [i [p [s]]]
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
# % genjson $d
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
# % genjson d
# dict d
# a ->
# ...

set ::LC "\{"
set ::RC "\}"
set ::LB "\["
set ::RB "\]"

proc genjson { d {i 0} {p "  "} {s " "} } {
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
        return -code error  "error: genjason - argument is not a dict"
    }
    set prefix "[string repeat $p $i]$::LC \""
    set max 0
    foreach key [dict keys $d] {
        if { [string length $key] > $max } {
            set max [string length $key]
        }
    }
    dict for {key val} ${d} {
        puts -nonewline "${prefix}[format "%-${max}s:" ${key}\"]$s"
        if {$fRepExist && [string match "value is a dict*" [tcl::unsupported::representation $val]]} {
#            puts ""
#            genjson $val [expr {$i+1}] $p $s
#            puts $::RC
             set val [dict2json $val]
             puts $val
        } else {
            if {[string is list $val] && ([llength $val] > 1)} {
              foreach entry $val {
                if {$fRepExist && [string is list $entry] && [expr {[llength $entry] % 2 == 0}] } {
#                  puts ""
#                  genjson $entry [expr {$i+1}] $p $s
#                  puts $::RC
                   set val [dict2json $entry]
                   puts $val
                } else {
                  puts "\"${entry}\""
                }
              }
            } else {
              puts "\"${val}\""
            }
        }
    }
    return
}

proc dict2json {dictVal} {
    # XXX: Currently this API isn't symmetrical, as to create proper
    # XXX: JSON text requires type knowledge of the input data
    set json ""
    set prefix ""

#puts stderr ">> $dictVal"
    foreach {key val} $dictVal {
        if {[string match "value is a dict*" [tcl::unsupported::representation $val]]} {
          set val [dict2json $val]
          if {$val eq [list]} {
            set val {""}
          }
          puts $val
        } else {
            if {[string is list $val] && ([llength $val] > 1)} {
              puts "$::LB"
              foreach entry $val {
                if {[string is list $entry] && [expr {[llength $entry] % 2 == 0}] } {
                  set val [dict2json $entry]
                  if {$val eq [list]} {
                    set val {""}
                  }
                  puts $val
                } else {
                  set val ${entry}
                  if {$val eq [list]} {
                    set val {""}
                  }
                  puts $val
                }
              }
              puts "$::RB"
            } else {
puts "\nval: $val![string is list $val]![llength $val]!"
	      # key must always be a string, val may be a number, string or
	      # bare word (true|false|null)
	      if {0 && ![string is double -strict $val]
	          && ![regexp {^(?:true|false|null)$} $val]} {
	          set val "\"$val\""
	      }
              if {$val eq ""} {
                set val {""}
              }
    	      append json "$prefix\"$key\": $val" \n
	      set prefix ,
            }
        }
    }

    return "\{${json}\}"
}