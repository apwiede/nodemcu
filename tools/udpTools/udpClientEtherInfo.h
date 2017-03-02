/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   udpClientEtherInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on March 01, 2017
 */

#ifndef UDP_CLIENT_ETHER_INFO_H
#define	UDP_CLIENT_ETHER_INFO_H


#define ETHERTYPE_LEN           2

#define ETHERTYPE_GRE_ISO       0x00FE  /* not really an ethertype only used in GRE */
#define	ETHERTYPE_PUP		0x0200	/* PUP protocol */
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#define ETHERTYPE_ARP		0x0806	/* Addr. resolution protocol */
#define ETHERTYPE_REVARP	0x8035	/* reverse Addr. resolution protocol */
#define ETHERTYPE_NS		0x0600
#define	ETHERTYPE_SPRITE	0x0500
#define ETHERTYPE_TRAIL		0x1000
#define	ETHERTYPE_MOPDL		0x6001
#define	ETHERTYPE_MOPRC		0x6002
#define	ETHERTYPE_DN		0x6003
#define	ETHERTYPE_LAT		0x6004
#define ETHERTYPE_SCA		0x6007
#define ETHERTYPE_TEB		0x6558
#define	ETHERTYPE_LANBRIDGE	0x8038
#define	ETHERTYPE_DECDNS	0x803c
#define	ETHERTYPE_DECDTS	0x803e
#define	ETHERTYPE_VEXP		0x805b
#define	ETHERTYPE_VPROD		0x805c
#define ETHERTYPE_ATALK		0x809b
#define ETHERTYPE_AARP		0x80f3
#define	ETHERTYPE_TIPC		0x88ca
#define	ETHERTYPE_8021Q		0x8100

/* see:
        http://en.wikipedia.org/wiki/IEEE_802.1Q
    and http://en.wikipedia.org/wiki/QinQ
*/
#define	ETHERTYPE_8021Q9100	0x9100
#define	ETHERTYPE_8021Q9200	0x9200
#define	ETHERTYPE_8021QinQ      0x88a8
#define ETHERTYPE_IPX		0x8137
#define ETHERTYPE_IPV6		0x86dd
#define	ETHERTYPE_PPP		0x880b
#define	ETHERTYPE_MPCP		0x8808
#define	ETHERTYPE_SLOW		0x8809
#define	ETHERTYPE_MPLS		0x8847
#define	ETHERTYPE_MPLS_MULTI	0x8848
#define ETHERTYPE_PPPOED	0x8863
#define ETHERTYPE_PPPOES	0x8864
#define ETHERTYPE_PPPOED2	0x3c12
#define ETHERTYPE_PPPOES2	0x3c13
#define ETHERTYPE_MS_NLB_HB	0x886f /* MS Network Load Balancing Heartbeat */
#define ETHERTYPE_JUMBO         0x8870
#define ETHERTYPE_LLDP          0x88cc
#define ETHERTYPE_EAPOL  	0x888e
#define ETHERTYPE_RRCP  	0x8899
#define ETHERTYPE_AOE  		0x88a2
#define	ETHERTYPE_LOOPBACK	0x9000
#define	ETHERTYPE_VMAN	        0x9100 /* Extreme VMAN Protocol */
#define	ETHERTYPE_CFM_OLD       0xabcd /* 802.1ag depreciated */
#define	ETHERTYPE_CFM           0x8902 /* 802.1ag */
#define	ETHERTYPE_IEEE1905_1    0x893a /* IEEE 1905.1 */
#define	ETHERTYPE_ISO           0xfefe  /* nonstandard - used in Cisco HDLC encapsulation */
#define	ETHERTYPE_CALM_FAST     0x1111  /* ISO CALM FAST */
#define	ETHERTYPE_GEONET_OLD    0x0707  /* ETSI GeoNetworking (before Jan 2013) */
#define	ETHERTYPE_GEONET        0x8947  /* ETSI GeoNetworking (Official IEEE registration from Jan 2013) */
#define	ETHERTYPE_MEDSA		0xdada	/* Marvel Distributed Switch Architecture */

#define ETHERMTU        1500

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define ETHER_ADDR_LEN  6

/*
 * Structure of an Ethernet header.
 */
struct ether_header {
  uint8_t   ether_dhost[ETHER_ADDR_LEN];
  uint8_t   ether_shost[ETHER_ADDR_LEN];
  uint16_t  ether_length_type;
};

#define ETHER_HDRLEN    14
#define BUFSIZE         128

extern const struct tok oui_values[];
extern const struct tok smi_values[];

#define OUI_ENCAP_ETHER       0x000000  /* encapsulated Ethernet */
#define OUI_CISCO             0x00000c  /* Cisco protocols */
#define OUI_IANA              0x00005E  /* IANA */
#define OUI_NORTEL            0x000081  /* Nortel SONMP */
#define OUI_CISCO_90          0x0000f8  /* Cisco bridging */
#define OUI_RFC2684           0x0080c2  /* RFC 2427/2684 bridged Ethernet */
#define OUI_ATM_FORUM         0x00A03E  /* ATM Forum */
#define OUI_CABLE_BPDU        0x00E02F  /* DOCSIS spanning tree BPDU */
#define OUI_APPLETALK         0x080007  /* Appletalk */
#define OUI_JUNIPER           0x009069  /* Juniper */
#define OUI_HP                0x080009  /* Hewlett-Packard */
#define OUI_IEEE_8021_PRIVATE 0x0080c2  /* IEEE 802.1 Organisation Specific - Annex F */
#define OUI_IEEE_8023_PRIVATE 0x00120f  /* IEEE 802.3 Organisation Specific - Annex G */
#define OUI_TIA               0x0012bb  /* TIA - Telecommunications Industry Association - ANSI/TIA-1057- 2006 */
#define OUI_DCBX              0x001B21  /* DCBX */
#define OUI_NICIRA            0x002320  /* Nicira Networks */
#define OUI_BSN               0x5c16c7  /* Big Switch Networks */
#define OUI_VELLO             0xb0d2f5  /* Vello Systems */
#define OUI_HP2               0x002481  /* HP too */
#define OUI_HPLABS            0x0004ea  /* HP-Labs */
#define OUI_INFOBLOX          0x748771  /* Infoblox Inc */
#define OUI_ONLAB             0xa42305  /* Open Networking Lab */
#define OUI_FREESCALE         0x00049f  /* Freescale */
#define OUI_NETRONOME         0x0015ad  /* Netronome */

#define HASHNAMESIZE 4096

struct enamemem {
  u_short e_addr0;
  u_short e_addr1;
  u_short e_addr2;
  const char *e_name;
  u_char *e_nsap;                 /* used only for nsaptable[] */
#define e_bs e_nsap                     /* for bytestringtable */
  struct enamemem *e_nxt;
};

struct hnamemem {
  uint32_t addr;
  const char *name;
  struct hnamemem *nxt;
};

typedef struct udpClientEtherInfo {
} udpClientEtherInfo_t;

#endif  /* UDP_CLIENT_ETHER_INFO_H */
