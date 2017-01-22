#!/usr/bin/env tclsh8.6

lappend auto_path [pwd]/lib
package require dwarfdbgclass
dwarfdbgclass create Dbg
catch {Dbg open /home/arnulf/bene-nodemcu-firmware/app/.output/eagle/debug/image/eagle.app.v6.0.out} MSG
puts stderr "MSG: $MSG!"
Dbg fcn1

