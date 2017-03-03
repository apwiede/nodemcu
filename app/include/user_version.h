#ifndef __USER_VERSION_H__
#define __USER_VERSION_H__

#define NODE_VERSION_MAJOR		1U
#define NODE_VERSION_MINOR		5U
#define NODE_VERSION_REVISION	1U
#define NODE_VERSION_INTERNAL   0U

#define NODE_VERSION "NodeMCU custom build by apwiede\n\tbranch: dev\n\tcommit: 95d48ada69d741146703c89282d058074ebd206b\n\tSSL: true\n\tmodules: compmsg,file,net,rboot,uart,wifi\n"
#ifndef BUILD_DATE
#define BUILD_DATE "\tbuilt on: 2017-02-24 14:49\n"
#endif

extern char SDK_VERSION[];

#endif	/* __USER_VERSION_H__ */
