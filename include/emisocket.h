/*
EMI:	embedded message interface
Copyright (C) 2009  Cooper <davidontech@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
*/

#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "emi.h"

struct sk_dpr {
	int d;
	int af;
};

union emi_sock_addr {
	struct sockaddr_in inet;
};


//#define GET_PORT (getuid(getuid()%SYSPORT+SYSPORT))
#define SYSPORT 1024
#define GET_PORT	({\
						long _u_id_=getuid();\
						_u_id_==0?EMI_PORT:_u_id_%SYSPORT+SYSPORT;\
					})



extern struct sk_dpr *emi_open(int addr_family);
extern void emi_close(struct sk_dpr *sd);
extern int emi_listen(struct sk_dpr *sd);
extern int emi_connect(struct sk_dpr *sd,struct emi_addr *dest_addr,eu32 retry);
extern int emicore_send_cmd(int sd,struct emi_msg *cmd);
extern int emi_bind(struct sk_dpr *sd,int port);
extern int emi_read(struct sk_dpr *sd,void *buf,eu32 size);
extern int emi_write(struct sk_dpr *sd,void *buf,eu32 size);
extern struct sk_dpr *emi_accept(struct sk_dpr *sd,union emi_sock_addr *addr);
#endif
