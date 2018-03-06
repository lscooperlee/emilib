#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "emi_msg.h"
#include "emi_sock.h"
#include "emi_sockr.h"
#include "emi_shbuf.h"
#include "emi_dbg.h"


int emi_listen(struct sk_dpr *sd){
    if(listen(sd->d,SOMAXCONN)<0){
        return -1;
    }
    return 0;
}

int emi_bind(struct sk_dpr *sd,int emi_port){
    union emi_sock_addr local_addr;
    switch(sd->af){
        case AF_INET:
            local_addr.inet.sin_family=AF_INET;
            local_addr.inet.sin_port=htons(emi_port);
            local_addr.inet.sin_addr.s_addr=htonl(INADDR_ANY);
            if(bind(sd->d,(struct sockaddr *)&local_addr.inet,sizeof(struct sockaddr_in))<0){
                return -1;
            }
            return 0;
        default:
            return -1;
    }
}

struct sk_dpr *emi_accept(struct sk_dpr *sd,union emi_sock_addr *addr){
    struct sk_dpr *sd_dest;
    socklen_t len;
    len=sizeof(union emi_sock_addr);
    if((sd_dest=(struct sk_dpr *)malloc(sizeof(struct sk_dpr)))==NULL)
        return NULL;
    if((sd_dest->d=accept(sd->d,(struct sockaddr *)addr,&len))<0){
        free(sd_dest);
        return NULL;
    }
    if(addr!=NULL){
        sd_dest->af=((struct sockaddr *)addr)->sa_family;
    }else{
        sd_dest->af=0;
    }
    return sd_dest;
}

static int emi_msg_write_retdata(struct sk_dpr *sd, struct emi_msg const *msg){
    struct emi_retdata *data;
    for_each_retdata(msg, data){
        if (emi_write(sd, data, data->size + sizeof(struct emi_retdata))) { 
            // send whole struct emi_retdata back
            return -1;
        }
    }
    return 0;
}

int emi_msg_write_ret(struct sk_dpr *sd, struct emi_msg const *msg){
    if (emi_msg_write_payload(sd, msg)){
        return -1;
    }
    emilog(EMI_DEBUG, "Write with retdata, retsize %d\n", msg->retsize);

    if (msg->retsize > 0) {
        return emi_msg_write_retdata(sd, msg);    
    }
    return 0;
}

static int emi_msg_read_data(struct sk_dpr *sd, struct emi_msg *msg){
    void *data = GET_ADDR(msg, msg->data_offset);
    if(emi_read(sd, data, msg->size)) {
        return -1;
    }
    return 0;
}

int emi_msg_read(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_msg_read_payload(sd, msg)){
        return -1;
    }

    if (msg->size > 0) {

        if(realloc_shared_msg(msg) == NULL){
            return -1;
        }

        if (emi_msg_read_data(sd, msg)) {
            return -1;
        }
        emilog(EMI_DEBUG, "Read with data, size %d\n", msg->size);
        
    }

    return 0;
}
