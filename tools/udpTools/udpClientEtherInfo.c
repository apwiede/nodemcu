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
 * File:   udpClientEtherInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on March 01, 2017
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

struct hnamemem hnametable[HASHNAMESIZE];
static struct hnamemem tporttable[HASHNAMESIZE];
static struct hnamemem uporttable[HASHNAMESIZE];
static struct hnamemem eprototable[HASHNAMESIZE];
static struct hnamemem dnaddrtable[HASHNAMESIZE];
static struct hnamemem ipxsaptable[HASHNAMESIZE];

static struct enamemem enametable[HASHNAMESIZE];
static struct enamemem nsaptable[HASHNAMESIZE];
static struct enamemem bytestringtable[HASHNAMESIZE];

struct protoidmem {
  uint32_t p_oui;
  u_short p_proto;
  const char *p_name;
  struct protoidmem *p_nxt;
};

static struct protoidmem protoidtable[HASHNAMESIZE];
static const char hex[] = "0123456789abcdef";


const struct tok ethertype_values[] = {
    { ETHERTYPE_IP,		"IPv4" },
    { ETHERTYPE_MPLS,		"MPLS unicast" },
    { ETHERTYPE_MPLS_MULTI,	"MPLS multicast" },
    { ETHERTYPE_IPV6,		"IPv6" },
    { ETHERTYPE_8021Q,		"802.1Q" },
    { ETHERTYPE_8021Q9100,	"802.1Q-9100" },
    { ETHERTYPE_8021QinQ,	"802.1Q-QinQ" },
    { ETHERTYPE_8021Q9200,	"802.1Q-9200" },
    { ETHERTYPE_VMAN,		"VMAN" },
    { ETHERTYPE_PUP,            "PUP" },
    { ETHERTYPE_ARP,            "ARP"},
    { ETHERTYPE_REVARP,         "Reverse ARP"},
    { ETHERTYPE_NS,             "NS" },
    { ETHERTYPE_SPRITE,         "Sprite" },
    { ETHERTYPE_TRAIL,          "Trail" },
    { ETHERTYPE_MOPDL,          "MOP DL" },
    { ETHERTYPE_MOPRC,          "MOP RC" },
    { ETHERTYPE_DN,             "DN" },
    { ETHERTYPE_LAT,            "LAT" },
    { ETHERTYPE_SCA,            "SCA" },
    { ETHERTYPE_TEB,            "TEB" },
    { ETHERTYPE_LANBRIDGE,      "Lanbridge" },
    { ETHERTYPE_DECDNS,         "DEC DNS" },
    { ETHERTYPE_DECDTS,         "DEC DTS" },
    { ETHERTYPE_VEXP,           "VEXP" },
    { ETHERTYPE_VPROD,          "VPROD" },
    { ETHERTYPE_ATALK,          "Appletalk" },
    { ETHERTYPE_AARP,           "Appletalk ARP" },
    { ETHERTYPE_IPX,            "IPX" },
    { ETHERTYPE_PPP,            "PPP" },
    { ETHERTYPE_MPCP,           "MPCP" },
    { ETHERTYPE_SLOW,           "Slow Protocols" },
    { ETHERTYPE_PPPOED,         "PPPoE D" },
    { ETHERTYPE_PPPOES,         "PPPoE S" },
    { ETHERTYPE_EAPOL,          "EAPOL" },
    { ETHERTYPE_RRCP,           "RRCP" },
    { ETHERTYPE_MS_NLB_HB,      "MS NLB heartbeat" },
    { ETHERTYPE_JUMBO,          "Jumbo" },
    { ETHERTYPE_LOOPBACK,       "Loopback" },
    { ETHERTYPE_ISO,            "OSI" },
    { ETHERTYPE_GRE_ISO,        "GRE-OSI" },
    { ETHERTYPE_CFM_OLD,        "CFM (old)" },
    { ETHERTYPE_CFM,            "CFM" },
    { ETHERTYPE_IEEE1905_1,     "IEEE1905.1" },
    { ETHERTYPE_LLDP,           "LLDP" },
    { ETHERTYPE_TIPC,           "TIPC"},
    { ETHERTYPE_GEONET_OLD,     "GeoNet (old)"},
    { ETHERTYPE_GEONET,         "GeoNet"},
    { ETHERTYPE_CALM_FAST,      "CALM FAST"},
    { ETHERTYPE_AOE,            "AoE" },
    { ETHERTYPE_MEDSA,          "MEDSA" },
    { 0, NULL}
};

#define TOKBUFSIZE 128

/* FIXME complete OUI list using a script */

const struct tok oui_values[] = {
    { OUI_ENCAP_ETHER, "Ethernet" },
    { OUI_CISCO, "Cisco" },
    { OUI_IANA, "IANA" },
    { OUI_NORTEL, "Nortel Networks SONMP" },
    { OUI_CISCO_90, "Cisco bridged" },
    { OUI_RFC2684, "Ethernet bridged" },
    { OUI_ATM_FORUM, "ATM Forum" },
    { OUI_CABLE_BPDU, "DOCSIS Spanning Tree" },
    { OUI_APPLETALK, "Appletalk" },
    { OUI_JUNIPER, "Juniper" },
    { OUI_HP, "Hewlett-Packard" },
    { OUI_IEEE_8021_PRIVATE, "IEEE 802.1 Private"},
    { OUI_IEEE_8023_PRIVATE, "IEEE 802.3 Private"},
    { OUI_TIA, "ANSI/TIA"},
    { OUI_DCBX, "DCBX"},
    { OUI_NICIRA, "Nicira Networks" },
    { OUI_BSN, "Big Switch Networks" },
    { OUI_VELLO, "Vello Systems" },
    { OUI_HP2, "HP" },
    { OUI_HPLABS, "HP-Labs" },
    { OUI_INFOBLOX, "Infoblox Inc" },
    { OUI_ONLAB, "Open Networking Lab" },
    { OUI_FREESCALE, "Freescale" },
    { OUI_NETRONOME, "Netronome" },
    { 0, NULL }
};

#define IP_RES 0x8000

static const struct tok ip_frag_values[] = {
        { IP_MF,        "+" },
        { IP_DF,        "DF" },
        { IP_RES,       "rsvd" }, /* The RFC3514 evil ;-) bit */
        { 0,            NULL }
};

struct ip_print_demux_state {
        const struct ip *ip;
        const u_char *cp;
        u_int   len, off;
        u_char  nh;
        int     advance;
};

static const char tstr[] = "[|ip]";

#define ipAddrString(ndo, p) getName(ndo, (const u_char *)(p))

static uint32_t f_netmask;
static uint32_t f_localnet;

extern void udpHandler(netdissect_options *ndo, register const u_char *bp, u_int length, register const u_char *bp2, int fragmented);

// =================================== lookupEmem =========================== 

/* Find the hash node that corresponds the ether address 'ep' */

static inline struct enamemem *lookupEmem(netdissect_options *ndo, const u_char *ep) {
  register u_int i, j, k;
  struct enamemem *tp;

  k = (ep[0] << 8) | ep[1];
  j = (ep[2] << 8) | ep[3];
  i = (ep[4] << 8) | ep[5];

  tp = &enametable[(i ^ j) & (HASHNAMESIZE-1)];
  while (tp->e_nxt)
    if (tp->e_addr0 == i &&
        tp->e_addr1 == j &&
        tp->e_addr2 == k)
      return tp;
    else
      tp = tp->e_nxt;
  tp->e_addr0 = i;
  tp->e_addr1 = j;
  tp->e_addr2 = k;
  tp->e_nxt = (struct enamemem *)calloc(1, sizeof(*tp));
  if (tp->e_nxt == NULL)
    (*ndo->ndo_error)(ndo, "lookupEmem: calloc");

  return tp;
}

// =================================== newhnamemem =========================== 

/* Return a zero'ed hnamemem struct and cuts down on calloc() overhead */
struct hnamemem *newhnamemem(netdissect_options *ndo) {
  register struct hnamemem *p;
  static struct hnamemem *ptr = NULL;
  static u_int num = 0;

  if (num  <= 0) {
    num = 64;
    ptr = (struct hnamemem *)calloc(num, sizeof (*ptr));
    if (ptr == NULL)
      (*ndo->ndo_error)(ndo, "newhnamemem: calloc");
  }
  --num;
  p = ptr++;
  return (p);
}

// =================================== intoa =========================== 

/*
 * A faster replacement for inet_ntoa().
 */
const char *intoa(uint32_t addr) {
  register char *cp;
  register u_int byte;
  register int n;
  static char buf[sizeof(".xxx.xxx.xxx.xxx")];

  NTOHL(addr);
  cp = buf + sizeof(buf);
  *--cp = '\0';

  n = 4;
  do {
    byte = addr & 0xff;
    *--cp = byte % 10 + '0';
    byte /= 10;
    if (byte > 0) {
      *--cp = byte % 10 + '0';
      byte /= 10;
      if (byte > 0)
        *--cp = byte + '0';
    }
    *--cp = '.';
    addr >>= 8;
  } while (--n > 0);

  return cp + 1;
}

// =================================== getName =========================== 

/*
 * Return a name for the IP address pointed to by ap.  This address
 * is assumed to be in network byte order.
 *
 * NOTE: ap is *NOT* necessarily part of the packet data (not even if
 * this is being called with the "ipaddr_string()" macro), so you
 * *CANNOT* use the ND_TCHECK{2}/ND_TTEST{2} macros on it.  Furthermore,
 * even in cases where it *is* part of the packet data, the caller
 * would still have to check for a null return value, even if it's
 * just printing the return value with "%s" - not all versions of
 * printf print "(null)" with "%s" and a null pointer, some of them
 * don't check for a null pointer and crash in that case.
 *
 * The callers of this routine should, before handing this routine
 * a pointer to packet data, be sure that the data is present in
 * the packet buffer.  They should probably do those checks anyway,
 * as other data at that layer might not be IP addresses, and it
 * also needs to check whether they're present in the packet buffer.
 */
const char *getName(netdissect_options *ndo, const u_char *ap) {
  register struct hostent *hp;
  uint32_t addr;
  struct hnamemem *p;

  memcpy(&addr, ap, sizeof(addr));
  p = &hnametable[addr & (HASHNAMESIZE-1)];
  for (; p->nxt; p = p->nxt) {
    if (p->addr == addr)
      return (p->name);
  }
  p->addr = addr;
  p->nxt = newhnamemem(ndo);

  /*
   * Print names unless:
   *  (1) -n was given.
   *      (2) Address is foreign and -f was given. (If -f was not
   *      given, f_netmask and f_localnet are 0 and the test
   *      evaluates to true)
   */
  if (!ndo->ndo_nflag &&
      (addr & f_netmask) == f_localnet) {
#ifdef HAVE_CASPER
    if (capdns != NULL) {
      hp = cap_gethostbyaddr(capdns, (char *)&addr, 4,
          AF_INET);
    } else
#endif
      hp = gethostbyaddr((char *)&addr, 4, AF_INET);
    if (hp) {
      char *dotp;

      p->name = strdup(hp->h_name);
      if (p->name == NULL)
        (*ndo->ndo_error)(ndo,
              "getname: strdup(hp->h_name)");
      if (ndo->ndo_Nflag) {
        /* Remove domain qualifications */
        dotp = strchr(p->name, '.');
        if (dotp)
          *dotp = '\0';
      }
      return (p->name);
    }
  }
  p->name = strdup(intoa(addr));
  if (p->name == NULL)
    (*ndo->ndo_error)(ndo, "getname: strdup(intoa(addr))");
  return (p->name);
}

// =================================== tok2strbuf =========================== 

/*
 * Convert a token value to a string; use "fmt" if not found.
 */
const char *tok2strbuf(register const struct tok *lp, register const char *fmt, register u_int v, char *buf, size_t bufsize) {
  if (lp != NULL) {
    while (lp->s != NULL) {
      if (lp->v == v) {
        return (lp->s);
      }
      ++lp;
    }
  }
  if (fmt == NULL)
    fmt = "#%d";

  (void)snprintf(buf, bufsize, fmt, v);
  return (const char *)buf;
}

// =================================== tok2str =========================== 

/*
 * Convert a token value to a string; use "fmt" if not found.
 */
const char * tok2str(register const struct tok *lp, register const char *fmt, register u_int v) {
  static char buf[4][TOKBUFSIZE];
  static int idx = 0;
  char *ret;

  ret = buf[idx];
  idx = (idx+1) & 3;
  return tok2strbuf(lp, fmt, v, ret, sizeof(buf[0]));
}


// =================================== ipHandleDemux =========================== 

static void ipHandleDemux(netdissect_options *ndo, struct ip_print_demux_state *ipds) {
  struct protoent *proto;

again:
  switch (ipds->nh) {
  case IPPROTO_UDP:
    /* pass on the MF bit plus the offset to detect fragments */
    udpHandler(ndo, ipds->cp, ipds->len, (const u_char *)ipds->ip, ipds->off & (IP_MF|IP_OFFMASK));
    break;
  default:
    if (ndo->ndo_nflag==0 && (proto = getprotobynumber(ipds->nh)) != NULL)
      ND_PRINT((ndo, " %s", proto->p_name));
    else
      ND_PRINT((ndo, " ip-proto-%d", ipds->nh));
    ND_PRINT((ndo, " %d", ipds->len));
    break;
  }
}

// =================================== ipHandle =========================== 

/*
 * handle an IP datagram.
 */
void ipHandle(netdissect_options *ndo, const u_char *bp, u_int length) {
  struct ip_print_demux_state  ipd;
  struct ip_print_demux_state *ipds=&ipd;
  const u_char *ipend;
  u_int hlen;
  struct cksum_vec vec[1];
  uint16_t sum, ip_sum;
  struct protoent *proto;

  ipds->ip = (const struct ip *)bp;
  ND_TCHECK(ipds->ip->ip_vhl);
  if (IP_V(ipds->ip) != 4) { /* print version and fail if != 4 */
      if (IP_V(ipds->ip) == 6)
        ND_PRINT((ndo, "IP6, wrong link-layer encapsulation "));
      else
        ND_PRINT((ndo, "IP%u ", IP_V(ipds->ip)));
      return;
  }
  if (!ndo->ndo_eflag)
    ND_PRINT((ndo, "IP "));

  ND_TCHECK(*ipds->ip);
  if (length < sizeof (struct ip)) {
    ND_PRINT((ndo, "truncated-ip %u", length));
    return;
  }
  hlen = IP_HL(ipds->ip) * 4;
  if (hlen < sizeof (struct ip)) {
    ND_PRINT((ndo, "bad-hlen %u", hlen));
    return;
  }

  ipds->len = EXTRACT_16BITS(&ipds->ip->ip_len);
  if (length < ipds->len)
    ND_PRINT((ndo, "truncated-ip - %u bytes missing! ", ipds->len - length));
  if (ipds->len < hlen) {
    ND_PRINT((ndo, "bad-len %u", ipds->len));
    return;
  }

  /*
   * Cut off the snapshot length to the end of the IP payload.
   */
  ipend = bp + ipds->len;
  if (ipend < ndo->ndo_snapend)
    ndo->ndo_snapend = ipend;

  ipds->len -= hlen;

  ipds->off = EXTRACT_16BITS(&ipds->ip->ip_off);


  /*
   * If this is fragment zero, hand it to the next higher
   * level protocol.
   */
  if ((ipds->off & 0x1fff) == 0) {
    ipds->cp = (const u_char *)ipds->ip + hlen;
    ipds->nh = ipds->ip->ip_p;

    if (ipds->nh != IPPROTO_TCP && ipds->nh != IPPROTO_UDP && ipds->nh != IPPROTO_SCTP && ipds->nh != IPPROTO_DCCP) {
      ND_PRINT((ndo, "%s > %s: ", ipAddrString(ndo, &ipds->ip->ip_src), ipAddrString(ndo, &ipds->ip->ip_dst)));
    }
    ipHandleDemux(ndo, ipds);
  } else {
    /*
     * This isn't the first frag, so we're missing the
     * next level protocol header.  print the ip addr
     * and the protocol.
     */
    ND_PRINT((ndo, "%s > %s:", ipAddrString(ndo, &ipds->ip->ip_src), ipAddrString(ndo, &ipds->ip->ip_dst)));
    if (!ndo->ndo_nflag && (proto = getprotobynumber(ipds->ip->ip_p)) != NULL)
      ND_PRINT((ndo, " %s", proto->p_name));
    else 
      ND_PRINT((ndo, " ip-proto-%d", ipds->ip->ip_p)); 
  }
  return;

trunc:
  ND_PRINT((ndo, "%s", tstr));
  return;
}

// =================================== etherTypeHandle =========================== 

/*
 * Prints the packet payload, given an Ethernet type code for the payload's
 * protocol.
 *
 * Returns non-zero if it can do so, zero if the ethertype is unknown.
 */
int etherTypeHandle(netdissect_options *ndo, u_short ether_type, const u_char *p,
      u_int length, u_int caplen, const struct lladdr_info *src, const struct lladdr_info *dst) {

  switch (ether_type) {
  case ETHERTYPE_IP:
    ipHandle(ndo, p, length);
    return (1);
  default:
    return (0);
  }
}

// =================================== etherAddrString =========================== 

const char *etherAddrString(netdissect_options *ndo, register const u_char *ep) {
  register int i;
  register char *cp;
  register struct enamemem *tp;
  int oui;
  char buf[BUFSIZE];

  tp = lookupEmem(ndo, ep);
  if (tp->e_name)
    return (tp->e_name);
#ifdef USE_ETHER_NTOHOST
  if (!ndo->ndo_nflag) {
    char buf2[BUFSIZE];

    if (ether_ntohost(buf2, (const struct ether_addr *)ep) == 0) {
      tp->e_name = strdup(buf2);
      if (tp->e_name == NULL)
        (*ndo->ndo_error)(ndo,
              "etherAddrString: strdup(buf2)");
      return (tp->e_name);
    }
  }
#endif
  cp = buf;
  oui = EXTRACT_24BITS(ep);
  *cp++ = hex[*ep >> 4 ];
  *cp++ = hex[*ep++ & 0xf];
  for (i = 5; --i >= 0;) {
    *cp++ = ':';
    *cp++ = hex[*ep >> 4 ];
    *cp++ = hex[*ep++ & 0xf];
  }

  if (!ndo->ndo_nflag) {
    snprintf(cp, BUFSIZE - (2 + 5*3), " (oui %s)",
    tok2str(oui_values, "Unknown", oui));
  } else
    *cp = '\0';
  tp->e_name = strdup(buf);
  if (tp->e_name == NULL)
    (*ndo->ndo_error)(ndo, "etherAddrString: strdup(buf)");
  return (tp->e_name);
}

// =================================== etherHdrHandle =========================== 

static inline void etherHdrHandle(netdissect_options *ndo, const u_char *bp, u_int length) {       
  register const struct ether_header *ep;
  uint16_t length_type;
        
  ep = (const struct ether_header *)bp;
        
  ND_PRINT((ndo, "%s > %s", etherAddrString(ndo, ESRC(ep)), etherAddrString(ndo, EDST(ep))));
  
  length_type = EXTRACT_16BITS(&ep->ether_length_type);
  if (!ndo->ndo_qflag) {
    if (length_type <= ETHERMTU) {
      ND_PRINT((ndo, ", 802.3"));
      length = length_type;
    } else  ;
      ND_PRINT((ndo, ", ethertype %s (0x%04x)", tok2str(ethertype_values,"Unknown", length_type), length_type));
  } else {
    if (length_type <= ETHERMTU) {
      ND_PRINT((ndo, ", 802.3"));
      length = length_type;
    } else 
      ND_PRINT((ndo, ", %s", tok2str(ethertype_values,"Unknown Ethertype (0x%04x)", length_type)));
  }

  ND_PRINT((ndo, ", length %u: ", length));
}

// =================================== etherHandler =========================== 

/*
 * Handle an Ethernet frame.
 * This might be encapsulated within another frame; we might be passed
 * a pointer to a function that can print header information for that
 * frame's protocol, and an argument to pass to that function.
 *
 * FIXME: caplen can and should be derived from ndo->ndo_snapend and p.
 */
typedef void (* handleEncapHeader_t)(netdissect_options *ndo, const u_char *);

u_int etherHandler(netdissect_options *ndo, const u_char *p, u_int length, u_int caplen,
  handleEncapHeader_t *handleEncapHeader, const u_char *encap_header_arg) {
  const struct ether_header *ep;
  u_int orig_length;
  u_short length_type;
  u_int hdrlen;
  int llc_hdrlen;
  struct lladdr_info src, dst;

  if (caplen < ETHER_HDRLEN) {
    ND_PRINT((ndo, "[|ether]"));
    return (caplen);
  }
  if (length < ETHER_HDRLEN) {
    ND_PRINT((ndo, "[|ether]"));
    return (length);
  }

  // eflag default is 0, handleEncapHeader is NULL
  if (ndo->ndo_eflag) {
    if (handleEncapHeader != NULL)
      (*handleEncapHeader)(ndo, encap_header_arg);
    etherHdrHandle(ndo, p, length);
  }
  orig_length = length;

  length -= ETHER_HDRLEN;
  caplen -= ETHER_HDRLEN;
  ep = (const struct ether_header *)p;
  p += ETHER_HDRLEN;
  hdrlen = ETHER_HDRLEN;

  src.addr = ESRC(ep);
  src.addr_string = etherAddrString;
  dst.addr = EDST(ep);
  dst.addr_string = etherAddrString;
  length_type = EXTRACT_16BITS(&ep->ether_length_type);

recurse:
  /*
   * Is it (gag) an 802.3 encapsulation?
   */
  if (length_type <= ETHERMTU) {
printf("ETHERMTU\n");
#ifdef NOTDEF
    /* Try to print the LLC-layer header & higher layers */
    llc_hdrlen = llc_print(ndo, p, length, caplen, &src, &dst);
    if (llc_hdrlen < 0) {
      /* packet type not known, print raw packet */
      if (!ndo->ndo_suppress_default_print)
        ND_DEFAULTPRINT(p, caplen);
      llc_hdrlen = -llc_hdrlen;
    }
    hdrlen += llc_hdrlen;
#endif
  } else if (length_type == ETHERTYPE_8021Q  ||
                length_type == ETHERTYPE_8021Q9100 ||
                length_type == ETHERTYPE_8021Q9200 ||
                length_type == ETHERTYPE_8021QinQ) {
printf("ETHERTYPE_8021Q*\n");
#ifdef NOTDEF
    /*
     * Print VLAN information, and then go back and process
     * the enclosed type field.
     */
    if (caplen < 4) {
      ND_PRINT((ndo, "[|vlan]"));
      return (hdrlen + caplen);
    }
    if (length < 4) {
      ND_PRINT((ndo, "[|vlan]"));
      return (hdrlen + length);
    }
          if (ndo->ndo_eflag) {
      uint16_t tag = EXTRACT_16BITS(p);

      ND_PRINT((ndo, "%s, ", ieee8021q_tci_string(tag)));
    }

    length_type = EXTRACT_16BITS(p + 2);
    if (ndo->ndo_eflag && length_type > ETHERMTU)
      ND_PRINT((ndo, "ethertype %s, ", tok2str(ethertype_values,"0x%04x", length_type)));
    p += 4;
    length -= 4;
    caplen -= 4;
    hdrlen += 4;
    goto recurse;
#endif
  } else if (length_type == ETHERTYPE_JUMBO) {
printf("ETHERTYPE_JUMBO\n");
#ifdef NOTDEF
    /*
     * Alteon jumbo frames.
     * See
     *
     *  http://tools.ietf.org/html/draft-ietf-isis-ext-eth-01
     *
     * which indicates that, following the type field,
     * there's an LLC header and payload.
     */
    /* Try to print the LLC-layer header & higher layers */
    llc_hdrlen = llc_print(ndo, p, length, caplen, &src, &dst);
    if (llc_hdrlen < 0) {
      /* packet type not known, print raw packet */
      if (!ndo->ndo_suppress_default_print)
        ND_DEFAULTPRINT(p, caplen);
      llc_hdrlen = -llc_hdrlen;
    }
    hdrlen += llc_hdrlen;
#endif
  } else {
    if (etherTypeHandle(ndo, length_type, p, length, caplen, &src, &dst) == 0) {
      /* type not known, print raw packet */
      if (!ndo->ndo_eflag) {
        if (handleEncapHeader != NULL)
          (*handleEncapHeader)(ndo, encap_header_arg);
        etherHdrHandle(ndo, (const u_char *)ep, orig_length);
      }

      if (!ndo->ndo_suppress_default_print) {
        ND_DEFAULTPRINT(p, caplen);
      }
    }
  }
  return (hdrlen);
}

// =================================== udpClienEtherInfoInit =========================== 

int udpClientEtherInfoInit (udpClientPtr_t self) {
  return UDP_CLIENT_ERR_OK;
}

