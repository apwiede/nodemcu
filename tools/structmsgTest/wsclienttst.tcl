#!/usr/bin/env tclsh8.6

package require websocket
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

source structmsgCmd.tcl

::websocket::loglevel debug

set PORT 8080
set path "/echo"
set host "192.168.178.42"

proc clientHandler { sock type msg } {
puts stderr "clientHandler: $type $msg!"
    switch -glob -nocase -- $type {
	co* {
	    puts "Connected on $sock"
	}
	te* {
	    puts "RECEIVED: $msg"
	}
	cl* -
	dis* {
	}
        binary {
            foreach val [split $msg "\0"] {
               puts stderr "val: $val!"
            }
        }
    }
    
}

proc encryptMsg {pwd} {
  set offset 0
  set handle [structmsg_create 5]
  structmsg_set_fieldValue $handle "@src" 123
  structmsg_set_fieldValue $handle "@dst" 456
  structmsg_set_fieldValue $handle "@cmdKey" 789
  structmsg_add_field $handle "@randomNum" uint32_t 4
  structmsg_add_field $handle "pwd" uint8_t* 16
  structmsg_set_fieldValue $handle "pwd" $pwd
  structmsg_set_fillerAndCrc $handle
  structmsg_encode $handle
  set encryptedMsg [structmsg_encrypt $handle "a1b2c3d4e5f6g7h8"]

#  structmsg_dump $handle
#  structmsg_dump $handle
#  puts stderr "DICT:"
#  pdict [set ::structmsg($handle)]
  return $encryptedMsg
}

proc test { sock } {
    after 2000
    puts stderr "[::websocket::conninfo $sock type] from [::websocket::conninfo $sock sockname] to [::websocket::conninfo $sock peername]"
puts stderr "===snd: binary: encrypted pwdMsg!"
    set encryptedMsg [encryptMsg "/dir1/dir2/dir34"]
puts stderr "encryptedMsg: $encryptedMsg!lgth: [string length $encryptedMsg]!"
    ::websocket::send $sock binary $encryptedMsg
#    ::websocket::send $sock text "load:McuMsg.lua"
}

set clientSocket [::websocket::open "ws://${host}:${PORT}${path}" ::clientHandler] 
puts stderr "===clientSocket: $clientSocket"

after 2000 test $::clientSocket

vwait forever
