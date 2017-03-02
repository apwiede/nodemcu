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
 * File:   udpClientUdpData.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on March 02, 2017
 */

#include <stdint.h>
#include <netdissect.h>
#include <netdissect-stdinc.h>
#include <sys/stat.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <tcl.h>
#include "udpClientInt.h"

//struct ip_print_demux_state {
//        const struct ip *ip;
//        const u_char *cp;
//        u_int   len, off;
//        u_char  nh;
//        int     advance;
//};

static const char udp_tstr[] = " [|udp]";
static struct hnamemem uporttable[HASHNAMESIZE];
extern struct hnamemem hnametable[];
extern struct hnamemem *newhnamemem(netdissect_options *ndo);

static uint32_t f_netmask;
static uint32_t f_localnet;


/*
 * Checksum routine for Internet Protocol family headers (Portable Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

#define ADDCARRY(x)  {if ((x) > 65535) (x) -= 65535;}
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}

extern const char *getName(netdissect_options *ndo, const u_char *ap);

#define ipAddrString(ndo, p) getName(ndo, (const u_char *)(p))

// =================================== inChksum =========================== 

uint16_t inCksum(const struct cksum_vec *vec, int veclen) {
  register const uint16_t *w;
  register int sum = 0;
  register int mlen = 0;
  int byte_swapped = 0;

  union {
    uint8_t    c[2];
    uint16_t  s;
  } s_util;
  union {
    uint16_t  s[2];
    uint32_t  l;
  } l_util;

  for (; veclen != 0; vec++, veclen--) {
    if (vec->len == 0)
      continue;
    w = (const uint16_t *)(const void *)vec->ptr;
    if (mlen == -1) {
      /*
       * The first byte of this chunk is the continuation
       * of a word spanning between this chunk and the
       * last chunk.
       *
       * s_util.c[0] is already saved when scanning previous
       * chunk.
       */
      s_util.c[1] = *(const uint8_t *)w;
      sum += s_util.s;
      w = (const uint16_t *)(const void *)((const uint8_t *)w + 1);
      mlen = vec->len - 1;
    } else
      mlen = vec->len;
    /*
     * Force to even boundary.
     */
    if ((1 & (uintptr_t) w) && (mlen > 0)) {
      REDUCE;
      sum <<= 8;
      s_util.c[0] = *(const uint8_t *)w;
      w = (const uint16_t *)(const void *)((const uint8_t *)w + 1);
      mlen--;
      byte_swapped = 1;
    }
    /*
     * Unroll the loop to make overhead from
     * branches &c small.
     */
    while ((mlen -= 32) >= 0) {
      sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
      sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
      sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
      sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
      w += 16;
    }
    mlen += 32;
    while ((mlen -= 8) >= 0) {
      sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
      w += 4;
    }
    mlen += 8;
    if (mlen == 0 && byte_swapped == 0)
      continue;
    REDUCE;
    while ((mlen -= 2) >= 0) {
      sum += *w++;
    }
    if (byte_swapped) {
      REDUCE;
      sum <<= 8;
      byte_swapped = 0;
      if (mlen == -1) {
        s_util.c[1] = *(const uint8_t *)w;
        sum += s_util.s;
        mlen = 0;
      } else
        mlen = -1;
    } else if (mlen == -1)
      s_util.c[0] = *(const uint8_t *)w;
  }
  if (mlen == -1) {
    /* The last mbuf has odd # of bytes. Follow the
       standard (the odd byte may be shifted left by 8 bits
       or not as determined by endian-ness of the machine) */
    s_util.c[1] = 0;
    sum += s_util.s;
  }
  REDUCE;
  return (~sum & 0xffff);
}

// =================================== inChksumShouldBe =========================== 

/*
 * Given the host-byte-order value of the checksum field in a packet
 * header, and the network-byte-order computed checksum of the data
 * that the checksum covers (including the checksum itself), compute
 * what the checksum field *should* have been.
 */
uint16_t inChksumShouldBe(uint16_t sum, uint16_t computed_sum) {
  uint32_t shouldbe;

  /*
   * The value that should have gone into the checksum field
   * is the negative of the value gotten by summing up everything
   * *but* the checksum field.
   *
   * We can compute that by subtracting the value of the checksum
   * field from the sum of all the data in the packet, and then
   * computing the negative of that value.
   *
   * "sum" is the value of the checksum field, and "computed_sum"
   * is the negative of the sum of all the data in the packets,
   * so that's -(-computed_sum - sum), or (sum + computed_sum).
   *
   * All the arithmetic in question is one's complement, so the
   * addition must include an end-around carry; we do this by
   * doing the arithmetic in 32 bits (with no sign-extension),
   * and then adding the upper 16 bits of the sum, which contain
   * the carry, to the lower 16 bits of the sum, and then do it
   * again in case *that* sum produced a carry.
   *
   * As RFC 1071 notes, the checksum can be computed without
   * byte-swapping the 16-bit words; summing 16-bit words
   * on a big-endian machine gives a big-endian checksum, which
   * can be directly stuffed into the big-endian checksum fields
   * in protocol headers, and summing words on a little-endian
   * machine gives a little-endian checksum, which must be
   * byte-swapped before being stuffed into a big-endian checksum
   * field.
   *
   * "computed_sum" is a network-byte-order value, so we must put
   * it in host byte order before subtracting it from the
   * host-byte-order value from the header; the adjusted checksum
   * will be in host byte order, which is what we'll return.
   */
  shouldbe = sum;
  shouldbe += ntohs(computed_sum);
  shouldbe = (shouldbe & 0xFFFF) + (shouldbe >> 16);
  shouldbe = (shouldbe & 0xFFFF) + (shouldbe >> 16);
  return shouldbe;
}

// =================================== ipFindDst =========================== 

/*
 * If source-routing is present and valid, return the final destination.
 * Otherwise, return IP destination.
 *
 * This is used for UDP and TCP pseudo-header in the checksum
 * calculation.
 */

static uint32_t ipFindDst(netdissect_options *ndo, const struct ip *ip) {
  int length;
  int len;
  const u_char *cp;
  uint32_t retval;

  cp = (const u_char *)(ip + 1);
  length = (IP_HL(ip) << 2) - sizeof(struct ip);

  for (; length > 0; cp += len, length -= len) {
    int tt;

    ND_TCHECK(*cp);
    tt = *cp;
    if (tt == IPOPT_EOL)
      break;
    else if (tt == IPOPT_NOP)
      len = 1;
    else {
      ND_TCHECK(cp[1]);
      len = cp[1];
      if (len < 2)
        break;
    }
    ND_TCHECK2(*cp, len);
    switch (tt) {

    case IPOPT_SSRR:
    case IPOPT_LSRR:
      if (len < 7)
        break;
      UNALIGNED_MEMCPY(&retval, cp + len - 4, 4);
      return retval;
    }
  }
trunc:
  UNALIGNED_MEMCPY(&retval, &ip->ip_dst, sizeof(uint32_t));
  return retval;
}

// =================================== nextProto4Chksum =========================== 

int nextProto4Cksum(netdissect_options *ndo, const struct ip *ip, const uint8_t *data, u_int len, u_int covlen, u_int next_proto) {
  struct phdr {
    uint32_t src;
    uint32_t dst;
    u_char mbz;
    u_char proto;
    uint16_t len;
  } ph;
  struct cksum_vec vec[2];

  /* pseudo-header.. */
  ph.len = htons((uint16_t)len);
  ph.mbz = 0;
  ph.proto = next_proto;
  UNALIGNED_MEMCPY(&ph.src, &ip->ip_src, sizeof(uint32_t));
  if (IP_HL(ip) == 5)
    UNALIGNED_MEMCPY(&ph.dst, &ip->ip_dst, sizeof(uint32_t));
  else
    ph.dst = ipFindDst(ndo, ip);

  vec[0].ptr = (const uint8_t *)(void *)&ph;
  vec[0].len = sizeof(ph);
  vec[1].ptr = data;
  vec[1].len = covlen;
  return (inCksum(vec, 2));
}

// =================================== udpChksum =========================== 

static int udpCksum(netdissect_options *ndo, register const struct ip *ip,
       register const struct udphdr *up, register u_int len) {
  return nextProto4Cksum(ndo, ip, (const uint8_t *)(const void *)up, len, len, IPPROTO_UDP);
}

// =================================== udpPortString =========================== 

const char * udpPortString(netdissect_options *ndo, register u_short port) {
  register struct hnamemem *tp;
  register uint32_t i = port;
  char buf[sizeof("00000")];

  for (tp = &uporttable[i & (HASHNAMESIZE-1)]; tp->nxt; tp = tp->nxt)
    if (tp->addr == i)
      return (tp->name);

  tp->addr = i;
  tp->nxt = newhnamemem(ndo);

  (void)snprintf(buf, sizeof(buf), "%u", i);
  tp->name = strdup(buf);
  if (tp->name == NULL)
    (*ndo->ndo_error)(ndo, "udpport_string: strdup(buf)");
  return (tp->name);
}

// =================================== udpIpAddrHandle =========================== 

static void udpIpAddrHandle(netdissect_options *ndo, const struct ip *ip, int sport, int dport) {
  const struct ip6_hdr *ip6;

  if (IP_V(ip) == 6)
    ip6 = (const struct ip6_hdr *)ip;
  else
    ip6 = NULL;

  if (ip6) {
  } else {
    if (ip->ip_p == IPPROTO_UDP) {
      if (sport == -1) {
        ND_PRINT((ndo, "%s > %s: ",
          ipAddrString(ndo, &ip->ip_src),
          ipAddrString(ndo, &ip->ip_dst)));
      } else {
        ND_PRINT((ndo, "%s.%s > %s.%s: ",
          ipAddrString(ndo, &ip->ip_src),
          udpPortString(ndo, sport),
          ipAddrString(ndo, &ip->ip_dst),
          udpPortString(ndo, dport)));
      }
    } else {
      if (sport != -1) {
        ND_PRINT((ndo, "%s > %s: ",
          udpPortString(ndo, sport),
          udpPortString(ndo, dport)));
      }
    }
  }
}

// =================================== udpHandler =========================== 

void udpHandler(netdissect_options *ndo, register const u_char *bp, u_int length, register const u_char *bp2, int fragmented) {
  register const struct udphdr *up;
  register const struct ip *ip;
  register const u_char *cp;
  register const u_char *ep = bp + length;
  uint16_t sport, dport, ulen;
  register const struct ip6_hdr *ip6;

  if (ep > ndo->ndo_snapend)
    ep = ndo->ndo_snapend;
  up = (const struct udphdr *)bp;
  ip = (const struct ip *)bp2;
  if (IP_V(ip) == 6)
    ip6 = (const struct ip6_hdr *)bp2;
  else
    ip6 = NULL;
  if (!ND_TTEST(up->uh_dport)) {
    udpIpAddrHandle(ndo, ip, -1, -1);
    goto trunc;
  }

  sport = EXTRACT_16BITS(&up->uh_sport);
  dport = EXTRACT_16BITS(&up->uh_dport);

  if (length < sizeof(struct udphdr)) {
    udpIpAddrHandle(ndo, ip, sport, dport);
    ND_PRINT((ndo, "truncated-udp %d", length));
    return;
  }
  if (!ND_TTEST(up->uh_ulen)) {
    udpIpAddrHandle(ndo, ip, sport, dport);
    goto trunc;
  }
  ulen = EXTRACT_16BITS(&up->uh_ulen);
  if (ulen < sizeof(struct udphdr)) {
    udpIpAddrHandle(ndo, ip, sport, dport);
    ND_PRINT((ndo, "truncated-udplength %d", ulen));
    return;
  }
  ulen -= sizeof(struct udphdr);
  length -= sizeof(struct udphdr);
  if (ulen < length)
    length = ulen;

  cp = (const u_char *)(up + 1);
  if (cp > ndo->ndo_snapend) {
    udpIpAddrHandle(ndo, ip, sport, dport);
    goto trunc;
  }

  if (ndo->ndo_packettype) {
    register const struct sunrpc_msg *rp;
//    enum sunrpc_msg_type direction;

    switch (ndo->ndo_packettype) {
    default:
      printf("WARNING no ndo->ndo_packettype printing\n");
    }
    return;
  }

//  udpipaddr_print(ndo, ip, sport, dport);
  if (!ndo->ndo_qflag) {
    register const struct sunrpc_msg *rp;
//    enum sunrpc_msg_type direction;

  }

  if (ndo->ndo_vflag && !ndo->ndo_Kflag && !fragmented) {
    /* Check the checksum, if possible. */
    uint16_t sum, udp_sum;

    /*
     * XXX - do this even if vflag == 1?
     * TCP does, and we do so for UDP-over-IPv6.
     */
          if (IP_V(ip) == 4 && (ndo->ndo_vflag > 1)) {
      udp_sum = EXTRACT_16BITS(&up->uh_sum);
      if (udp_sum == 0) {
        ND_PRINT((ndo, "[no cksum] "));
      } else if (ND_TTEST2(cp[0], length)) {
        sum = udpCksum(ndo, ip, up, length + sizeof(struct udphdr));

                          if (sum != 0) {
          ND_PRINT((ndo, "[bad udp cksum 0x%04x -> 0x%04x!] ",
              udp_sum,
              inChksumShouldBe(udp_sum, sum)));
        } else
          ND_PRINT((ndo, "[udp sum ok] "));
      }
    }
  }

  if (!ndo->ndo_qflag) {
    if (IS_SRC_OR_DST_PORT(NAMESERVER_PORT)) ;
//      ns_print(ndo, (const u_char *)(up + 1), length, 0);
    else if (IS_SRC_OR_DST_PORT(MULTICASTDNS_PORT)) ;
//      ns_print(ndo, (const u_char *)(up + 1), length, 1);
    else if (IS_SRC_OR_DST_PORT(TIMED_PORT)) ;
//      timed_print(ndo, (const u_char *)(up + 1));
    else if (IS_SRC_OR_DST_PORT(TFTP_PORT)) ;
//      tftp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(BOOTPC_PORT) || IS_SRC_OR_DST_PORT(BOOTPS_PORT)) ;
//      bootp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(RIP_PORT)) ;
//      rip_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(AODV_PORT)) ;
//      aodv_print(ndo, (const u_char *)(up + 1), length,
//          ip6 != NULL);
          else if (IS_SRC_OR_DST_PORT(ISAKMP_PORT)) ;
//       isakmp_print(ndo, (const u_char *)(up + 1), length, bp2);
          else if (IS_SRC_OR_DST_PORT(ISAKMP_PORT_NATT)) ;
//       isakmp_rfc3948_print(ndo, (const u_char *)(up + 1), length, bp2);
#if 1 /*???*/
          else if (IS_SRC_OR_DST_PORT(ISAKMP_PORT_USER1) || IS_SRC_OR_DST_PORT(ISAKMP_PORT_USER2)) ;
//      isakmp_print(ndo, (const u_char *)(up + 1), length, bp2);
#endif
    else if (IS_SRC_OR_DST_PORT(SNMP_PORT) || IS_SRC_OR_DST_PORT(SNMPTRAP_PORT)) ;
//      snmp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(NTP_PORT)) ;
//      ntp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(KERBEROS_PORT) || IS_SRC_OR_DST_PORT(KERBEROS_SEC_PORT)) ;
//      krb_print(ndo, (const void *)(up + 1));
    else if (IS_SRC_OR_DST_PORT(L2TP_PORT)) ;
//      l2tp_print(ndo, (const u_char *)(up + 1), length);
#ifdef ENABLE_SMB
    else if (IS_SRC_OR_DST_PORT(NETBIOS_NS_PORT)) ;
//      nbt_udp137_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(NETBIOS_DGRAM_PORT)) ;
//      nbt_udp138_print(ndo, (const u_char *)(up + 1), length);
#endif
    else if (dport == VAT_PORT) ;
//      vat_print(ndo, (const void *)(up + 1), up);
    else if (IS_SRC_OR_DST_PORT(ZEPHYR_SRV_PORT) || IS_SRC_OR_DST_PORT(ZEPHYR_CLT_PORT)) ;
//      zephyr_print(ndo, (const void *)(up + 1), length);
    /*
     * Since there are 10 possible ports to check, I think
     * a <> test would be more efficient
     */
    else if ((sport >= RX_PORT_LOW && sport <= RX_PORT_HIGH) ||
       (dport >= RX_PORT_LOW && dport <= RX_PORT_HIGH)) ;
//      rx_print(ndo, (const void *)(up + 1), length, sport, dport,
//         (const u_char *) ip);
    else if (IS_SRC_OR_DST_PORT(RIPNG_PORT)) ;
//      ripng_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(DHCP6_SERV_PORT) || IS_SRC_OR_DST_PORT(DHCP6_CLI_PORT)) ;
//      dhcp6_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(AHCP_PORT)) ;
//      ahcp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(BABEL_PORT) || IS_SRC_OR_DST_PORT(BABEL_PORT_OLD)) ;
//      babel_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(HNCP_PORT)) ;
//      hncp_print(ndo, (const u_char *)(up + 1), length);
    /*
     * Kludge in test for whiteboard packets.
     */
    else if (dport == WB_PORT) ;
//      wb_print(ndo, (const void *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(CISCO_AUTORP_PORT)) ;
//      cisco_autorp_print(ndo, (const void *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(RADIUS_PORT) ||
       IS_SRC_OR_DST_PORT(RADIUS_NEW_PORT) ||
       IS_SRC_OR_DST_PORT(RADIUS_ACCOUNTING_PORT) ||
       IS_SRC_OR_DST_PORT(RADIUS_NEW_ACCOUNTING_PORT) ||
       IS_SRC_OR_DST_PORT(RADIUS_CISCO_COA_PORT) ||
       IS_SRC_OR_DST_PORT(RADIUS_COA_PORT) ) ;
//      radius_print(ndo, (const u_char *)(up+1), length);
    else if (dport == HSRP_PORT) ;
//      hsrp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(LWRES_PORT)) ;
//      lwres_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(LDP_PORT)) ;
//      ldp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(OLSR_PORT)) ;
//      olsr_print(ndo, (const u_char *)(up + 1), length,
//          (IP_V(ip) == 6) ? 1 : 0);
    else if (IS_SRC_OR_DST_PORT(MPLS_LSP_PING_PORT)) ;
//      lspping_print(ndo, (const u_char *)(up + 1), length);
    else if (dport == BFD_CONTROL_PORT ||
       dport == BFD_ECHO_PORT ) ;
//      bfd_print(ndo, (const u_char *)(up+1), length, dport);
                else if (IS_SRC_OR_DST_PORT(LMP_PORT)) ;
//      lmp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(VQP_PORT)) ;
//      vqp_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(SFLOW_PORT)) ;
//                        sflow_print(ndo, (const u_char *)(up + 1), length);
          else if (dport == LWAPP_CONTROL_PORT) ;
//      lwapp_control_print(ndo, (const u_char *)(up + 1), length, 1);
                else if (sport == LWAPP_CONTROL_PORT) ;
 //                       lwapp_control_print(ndo, (const u_char *)(up + 1), length, 0);
                else if (IS_SRC_OR_DST_PORT(LWAPP_DATA_PORT)) ;
  //                      lwapp_data_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(SIP_PORT)) ;
//      sip_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(SYSLOG_PORT)) ;
//      syslog_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(OTV_PORT)) ;
//      otv_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(VXLAN_PORT)) ;
//      vxlan_print(ndo, (const u_char *)(up + 1), length);
                else if (IS_SRC_OR_DST_PORT(GENEVE_PORT)) ;
//      geneve_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(LISP_CONTROL_PORT)) ;
//      lisp_print(ndo, (const u_char *)(up + 1), length);
    else if (IS_SRC_OR_DST_PORT(VXLAN_GPE_PORT)) ;
//      vxlan_gpe_print(ndo, (const u_char *)(up + 1), length);
    else if (ND_TTEST(((const struct LAP *)cp)->type) &&
        ((const struct LAP *)cp)->type == lapDDP
//        && (atalk_port(sport) || atalk_port(dport))
        ) {
      if (ndo->ndo_vflag)
        ND_PRINT((ndo, "kip "));
//      llap_print(ndo, cp, length);
    } else {
      if (ulen > length)
        ND_PRINT((ndo, "UDP, bad length %u > %u",
            ulen, length));
      else
        ND_PRINT((ndo, "UDP, length %u", ulen));
    }
  } else {
    if (ulen > length)
      ND_PRINT((ndo, "UDP, bad length %u > %u",
          ulen, length));
    else
      ND_PRINT((ndo, "UDP, length %u", ulen));
  }
  return;

trunc:
  ND_PRINT((ndo, "%s", udp_tstr));
}

// =================================== udpClienUdpDataInit =========================== 

int udpClientUdpDataInit (udpClientPtr_t self) {
  return UDP_CLIENT_ERR_OK;
}

