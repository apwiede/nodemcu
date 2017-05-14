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
        if {$fRepExist && [string match "value is a dict*" [tcl::unsupported::representation $val]]} {
            puts ""
            pdict $val [expr {$i+1}] $p $s
        } else {
            if {[string is list $val] && ([llength $val] > 1)} {
              set listIdx 0
              foreach entry $val {
                if {$listIdx == 0} {
#                  puts "Z[string length $p]"
                  puts ""
                }
                if {$fRepExist && [string is list $entry] && [expr {[llength $entry] % 2 == 0}] } {
#                  puts "$listIdx: "
                  puts "${prefix} $listIdx: "
                  pdict $entry [expr {$i+1}] $p $s
                } else {
#                  puts "'$listIdx: ${entry}'"
                  puts "${prefix} $listIdx: '${entry}'"
                }
                incr listIdx
              }
            } else {
              puts "'${val}'"
            }
        }
    }
    return
}

