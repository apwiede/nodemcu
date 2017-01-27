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
  dD closeElf
} MSG]} {
  puts stderr "MSG: $MSG!"
}

