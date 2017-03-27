
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "emi_msg.h"

struct sk_dpr {
    int d;
    int af;
};

union emi_sock_addr {
    struct sockaddr_in inet;
};


#define SYSPORT 1024
#define GET_PORT    ({\
                        long _u_id_=getuid();\
                        _u_id_==0?EMI_PORT:_u_id_%SYSPORT+SYSPORT;\
                    })



extern struct sk_dpr *emi_open(int addr_family);
extern void emi_close(struct sk_dpr *sd);
extern int emi_listen(struct sk_dpr *sd);
extern int emi_connect(struct sk_dpr *sd,struct emi_addr *dest_addr,eu32 retry);
extern int emi_bind(struct sk_dpr *sd,int port);
extern int emi_read(struct sk_dpr *sd,void *buf,eu32 size);
extern int emi_write(struct sk_dpr *sd,void *buf,eu32 size);
extern struct sk_dpr *emi_accept(struct sk_dpr *sd,union emi_sock_addr *addr);

extern int emi_msg_write_payload(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_write_data(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_write(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_write_ret(struct sk_dpr *sd, struct emi_msg *msg);

extern int emi_msg_read_payload(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_read_data(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_read(struct sk_dpr *sd, struct emi_msg *msg);
extern int emi_msg_read_ret(struct sk_dpr *sd, struct emi_msg *msg);
#endif
