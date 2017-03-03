#!/usr/bin/env tclsh8.6

if {[llength $argv ] < 1} {
    puts stderr "usage: $argv0 <file name>"
    exit 1
}
set fileName [lindex $argv 0]
puts stderr "PWD: [pwd]!$fileName"
set fd [open $fileName r]
while {[gets $fd line] >= 0} {
    switch -glob -- $line {
    "" -
    "#*" {
        continue
    }
    }
    lappend lst $line
}
close $fd

set in_file_name "app/include/user_modules.h"
set out_file_name "app/include/user_modules.h.tmp"
set in_fd [open $in_file_name r]
set out_fd [open $out_file_name w]
set start false
while {[gets $in_fd line] >= 0} {
    switch -glob -- $line {
      "*#define LUA_USE_MODULES_*" {
          if {!$start} {
              set start true
          }
      }
      "*#endif  /* LUA_CROSS_COMPILER */" {
          set start false
          foreach tag $lst {
              puts $out_fd "#define LUA_USE_MODULES_[ string toupper $tag]"
          }
          puts $out_fd ""
          puts $out_fd $line
      }
      default {
         if {!$start} {
             puts $out_fd $line
         }
      }
    }
}
close $in_fd
flush $out_fd
close $out_fd
file rename -force $out_file_name $in_file_name
set fd [open "X_MODULES" w]
puts $fd [join $lst ","]
close $fd
