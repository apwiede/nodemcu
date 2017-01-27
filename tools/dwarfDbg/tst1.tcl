#!/usr/bin/env tclsh8.6

lappend auto_path [pwd]/lib
package require dwarfDbgClass
dwarfDbgClass create dD
if {[catch {
  dD init
  dD openElf /home/arnulf/bene-nodemcu-firmware/app/.output/eagle/debug/image/eagle.app.v6.0.out
  dD getDbgInfos
flush stderr
  set compileUnitNames [dD getFileInfos]
  # each entry has the follwing sub entries
  # compileUnitFileName compileUnitIdx filenameIdx numFileInfo numFileLine
puts "compileUnitNames: [join [lsort -unique $compileUnitNames] \n]"
flush stdout
  set idx 0
#  while {$idx < [llength $compileUnitNames]} {
    set fileLinesDict [dD getFileLines $idx]

set addressesDict [dict get $fileLinesDict addresses]
foreach key [dict keys $addressesDict] {
  set value [dict get $addressesDict $key]
puts stderr [format "  0x%08x %d" $key $value]
}

set linesDict [dict get $fileLinesDict lines]
foreach key [dict keys $linesDict] {
  set value [dict get $linesDict $key]
puts stderr [format "  %d 0x%08x" $key $value]
}

#puts "fileLines: $fileLinesDict"
#flush stdout
#    incr idx
#  }
  dD closeElf
} MSG]} {
  puts stderr "MSG: $MSG!"
}

