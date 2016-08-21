#!/usr/bin/env tclsh8.6

package require websocket
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

source structmsgCmd.tcl

::websocket::loglevel debug

set PORT 8080
set path "/echo"
set clients [list]

set srvSock [socket -server handleConnect $PORT]
::websocket::server $::srvSock
::websocket::live $::srvSock $::path wsLiveCB

proc decryptMsg {payload} {
  set offset 0
  set handle [structmsg_create 5]
  structmsg_set_fieldValue $handle "@src" 123
  structmsg_set_fieldValue $handle "@dst" 456
  structmsg_set_fieldValue $handle "@cmdKey" 789
  structmsg_add_field $handle "@randomNum" uint32_t 4
  structmsg_add_field $handle "pwd" uint8_t* 16
  structmsg_set_fillerAndCrc $handle
  set decryptedMsg [structmsg_decrypt $handle "a1b2c3d4e5f6g7h8" $payload]
puts stderr "decryptMsg2 len: [string length $decryptedMsg]!len2: [string length $payload]!"
#  structmsg_dump $handle
  structmsg_decode $handle $decryptedMsg
  structmsg_dump $handle
#  puts stderr "DICT:"
#  pdict [set ::structmsg($handle)]
  structmsg_get_fieldValue $handle pwd pwd
puts stderr "PWD: $pwd!"
  structmsg_get_fieldValue $handle @randomNum value
puts stderr "randomNum: [format 0x%08x $value]!"
  structmsg_get_fieldValue $handle @filler value
puts stderr "filler: $value!"
  structmsg_get_fieldValue $handle @crc value
puts stderr "crc: [format 0x%04x $value]!"
  structmsg_get_fieldValue $handle @src value
puts stderr "src: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @dst value
puts stderr "dst: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @totalLgth value
puts stderr "totalLgth: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @cmdKey value
puts stderr "cmdKey: [format {%d 0x%02x} $value $value]!"
}

proc sendToClient { str } {
    foreach cs $::clients {
        ::websocket::send $cs 1 "$str\n" 1
    }
}

proc handleConnect {cs addr p} {
puts stderr "handleConnect: cs: $cs addr: $addr p: $p!"
    if {[lsearch $::clients $cs] < 0} {
        lappend ::clients $cs
    }
    fileevent $cs readable [list handleRead $cs]
}
    
proc handleRead {cs} {
puts stderr "handleRead"
    set key [read $cs 1]
if {$key eq ""} {
    return
}
puts stderr "key: $key!"
    gets $cs request
set request ${key}$request
puts stderr "request: $request!"
    set path [lindex [split $request] 1]
    set hdrs [list]
    while { [gets $cs hdr] > 0 } {
        foreach { k v } $hdr break
        set k [string trim $k ":"]
        lappend hdrs $k $v
    }
puts stderr "hdrs: $hdrs!"
    set res [::websocket::test $::srvSock $cs $path $hdrs]
puts stderr "test: res: $res!"
    if {$res} {
        ::websocket::upgrade $cs
    }
puts stderr "path: $path!"
    return 
}

proc wsLiveCB {cs op par args} {
puts stderr "wsLiveCB op: $op!"
    switch $op { 
      request {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
      }
      close {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
          set idx [lsearch $::clients $cs]
          if { $idx >= 0 } {
              set ::clients [lreplace $::clients $idx $idx]
          }
      }
      disconnect {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
      }
      text {
puts stderr "wsLiveCB: $op $par"
        if {$par != "Message Received"} {
          set str "GOT message: $par!"
          ::websocket::send $cs 1 "$str\n" 1
        }
      } 
      binary {
puts stderr "op: $op! cs: $cs!"
puts stderr "par: $par!"
puts stderr "args: $args!"
set idx 0
foreach ch [split $par ""] {
  binary scan $ch c pch
  set pch [expr {$pch & 0xFF}]
#puts stderr [format "idx: $idx pch: 0x%02x!" $pch]
  incr idx
}
        decryptMsg $par
      }
    } 
}

vwait forever
