#!/usr/bin/env tclsh8.6

package require aes
set fd [open "CDTelegram.txt" r]
fconfigure $fd -translation binary
set fullMsg [read $fd]
close $fd

puts stderr "msg: ll: [string length $fullMsg]!"
set header [string range $fullMsg 0 31]
puts stderr "header: LL: [string length $header]!$header!"
set encrypted [string range $fullMsg 32 end-1]
set totalCrc [string range $fullMsg end end]
puts stderr "encrypted: LL: [string length $encrypted]!"
set key "a1b2c3d4e5f6g7h8"
set iv $key
set buf [aes::aes -mode cbc -dir decrypt -key $key -iv $iv $encrypted]
fconfigure stdout -translation binary
puts -nonewline ${header}${buf}${totalCrc}
