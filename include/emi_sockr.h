
#ifndef __EMI_SOCKR_H__
#define __EMI_SOCKR_H__

#include "emi_msg.h"
#include "emi_sock.h"

extern int emi_listen(struct sk_dpr *sd);
extern int emi_bind(struct sk_dpr *sd,int emi_port);
extern struct sk_dpr *emi_accept(struct sk_dpr *sd,union emi_sock_addr *addr);

extern int emi_msg_write_ret(struct sk_dpr *sd, struct emi_msg const *msg);

extern int emi_msg_read(struct sk_dpr *sd, struct emi_msg *msg);
#endif
