
#ifndef __EMI_SOCK_H__
#define __EMI_SOCK_H__

#include "emi_msg.h"

struct sk_dpr {
    int d;
    int af;
};

union emi_sock_addr {
    struct sockaddr_in inet;
};


extern struct sk_dpr *emi_open(int addr_family);
extern void emi_close(struct sk_dpr *sd);
extern int emi_connect(struct sk_dpr *sd,struct emi_addr *dest_addr);
extern int emi_read(struct sk_dpr *sd,void *buf,eu32 size);
extern int emi_write(struct sk_dpr *sd,void *buf,eu32 size);

extern int emi_msg_write_payload(struct sk_dpr *sd, struct emi_msg const *msg);
extern int emi_msg_write(struct sk_dpr *sd, struct emi_msg const *msg);

extern int emi_msg_read_payload(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_read_ret(struct sk_dpr *sd, struct emi_msg *msg);

#endif
