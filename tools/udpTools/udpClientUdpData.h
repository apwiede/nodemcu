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
 * File:   udpClientUdpdata.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on March 01, 2017
 */

#ifndef UDP_CLIENT_UDP_DATA_H
#define	UDP_CLIENT_Udp_DATA_H

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
struct udphdr {
        uint16_t        uh_sport;               /* source port */
        uint16_t        uh_dport;               /* destination port */
        uint16_t        uh_ulen;                /* udp length */
        uint16_t        uh_sum;                 /* udp checksum */
};

#define NAMESERVER_PORT                 53
#define TACACS_DB_PORT                  65      /*XXX*/
#define ORACLE_SQLNET_PORT              66      /*XXX*/
#define BOOTPS_PORT                     67      /* RFC951 */
#define BOOTPC_PORT                     68      /* RFC951 */
#define TFTP_PORT                       69      /*XXX*/
#define KERBEROS_PORT                   88      /*XXX*/
#define SUNRPC_PORT                     111     /*XXX*/
#define NTP_PORT                        123     /*XXX*/
#define NETBIOS_NS_PORT                 137     /* RFC 1001, RFC 1002 */
#define NETBIOS_DGRAM_PORT              138     /* RFC 1001, RFC 1002 */
#define NETBIOS_SSN_PORT                139     /* RFC 1001, RFC 1002 */
#define SNMP_PORT                       161     /*XXX*/
#define SNMPTRAP_PORT                   162     /*XXX*/
#define BGP_PORT                        179     /*XXX*/
#define APPLETALK_RTMP_PORT             201     /*XXX*/
#define APPLETALK_NB_PORT               202     /*XXX*/
#define APPLETALK_ECHO                  204     /*XXX*/
#define APPLETALK_ZONE_INFO_PORT        206     /*XXX*/
#define LDAP_PORT                       389     /*XXX*/
#define HTTPS_PORT                      443     /*XXX*/
#define MICROSOFT_DS_PORT               445     /*XXX*/
#define KERBEROS5_PASSWD_PORT           464     /* PER IANA */
#define CISCO_AUTORP_PORT               496     /*XXX*/
#define ISAKMP_PORT                     500     /*XXX*/
#define SYSLOG_PORT                     514     /* rfc3164 */
#define RIP_PORT                        520     /*XXX*/
#define RIPNG_PORT                      521     /* RFC 2080 */
#define TIMED_PORT                      525     /*XXX*/
#define KERBEROS_LOGIN_PORT             543     /*XXX*/
#define KERBEROS_SHELL_PORT             544     /*XXX*/
#define DHCP6_SERV_PORT                 546     /*XXX*/
#define DHCP6_CLI_PORT                  547     /*XXX*/
#define LDAPS_PORT                      636     /*XXX - LDAP over TLS/SSL */
#define LDP_PORT                        646
#define DHCP_FAILOVER_PORT              647     /*XXX*/
#define AODV_PORT                       654     /*XXX*/
#define OLSR_PORT                       698     /* rfc3626 */
#define LMP_PORT                        701     /* rfc4204 */
#define CISCO_TDP_PORT                  711     /*XXX*/
#define KERBEROS_ADM_PORT               749     /*XXX - Kerberos v5 */
#define KERBEROS_SEC_PORT               750     /*XXX - Kerberos v4 */
#define RSYNC_PORT                      873     /*XXX*/
#define LWRES_PORT                      921     /*XXX*/
#define OPENSSL_PORT                    1194    /*XXX*/
#define LOTUS_NOTES_PORT                1352    /*XXX*/
#define MS_SQL_SERVER_PORT              1433    /*XXX*/
#define MS_SQL_SERVER_MONITOR           1434    /*XXX*/
#define INGRESLOCK_PORT                 1524    /*XXX*/
#define VQP_PORT			1589	/*XXX*/
#define RADIUS_PORT			1645	/*XXX*/
#define RADIUS_ACCOUNTING_PORT		1646
#define RADIUS_CISCO_COA_PORT		1700
#define L2TP_PORT			1701	/*XXX*/
#define RADIUS_NEW_PORT			1812	/*XXX*/
#define RADIUS_NEW_ACCOUNTING_PORT	1813
#define HSRP_PORT			1985	/*XXX*/
#define NFS_DAEMON_PORT			2049	/*XXX*/
#define ZEPHYR_SRV_PORT			2103	/*XXX*/
#define ZEPHYR_CLT_PORT			2104	/*XXX*/
#define MYSQL_PORT			3306	/*XXX*/
#define MS_RDP_PORT			3389	/*XXX*/
#define VAT_PORT			3456	/*XXX*/
#define MPLS_LSP_PING_PORT		3503	/* draft-ietf-mpls-lsp-ping-02.txt */
#define SUBVERSION_PORT			3690	/*XXX*/
#define BFD_CONTROL_PORT		3784	/* RFC 5881 */
#define BFD_ECHO_PORT			3785	/* RFC 5881 */
#define RADIUS_COA_PORT			3799	/* RFC 5176 */
#define NFS_LOCK_DAEMON_PORT		4045	/*XXX*/
#define LISP_CONTROL_PORT		4342	/* RFC 6830 */
#define ISAKMP_PORT_NATT		4500	/* rfc3948 */
#define WB_PORT				4567
#define VXLAN_PORT			4789	/* RFC 7348 */
#define VXLAN_GPE_PORT			4790	/* draft-ietf-nvo3-vxlan-gpe-01 */
#define SIP_DS_PORT			5059	/*XXX*/
#define SIP_PORT			5060
#define MULTICASTDNS_PORT		5353	/* RFC 6762 */
#define AHCP_PORT			5359	/* draft-chroboczek-ahcp-00 */
#define GENEVE_PORT			6081	/* draft-gross-geneve-02 */
#define SFLOW_PORT			6343	/* http://www.sflow.org/developers/specifications.php */
#define BABEL_PORT			6696	/* RFC 6126 errata */
#define BABEL_PORT_OLD			6697	/* RFC 6126 */
#define RX_PORT_LOW			7000	/*XXX*/
#define RX_PORT_HIGH			7009	/*XXX*/
#define ISAKMP_PORT_USER1		7500	/*XXX - nonstandard*/
#define HNCP_PORT			8231	/* RFC 7788 */
#define OTV_PORT			8472	/* draft-hasmit-otv-04 */
#define ISAKMP_PORT_USER2		8500	/*XXX - nonstandard*/
#define LWAPP_DATA_PORT			12222	/* RFC 5412 */
#define LWAPP_CONTROL_PORT		12223	/* RFC 5412 */

struct LAP {
        uint8_t         dst;
        uint8_t         src;
        uint8_t         type;
};

#define lapShortDDP     1       /* short DDP type */
#define lapDDP          2       /* DDP type */
#define lapKLAP         'K'     /* Kinetics KLAP type */


typedef struct udpClientUdpData {
} udpClientUdpData_t;

#endif  /* UDP_CLIENT_UDP_DATA_H */
