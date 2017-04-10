#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

#include "emi_msg.h"
#include "emi_sock.h"
#include "emi_if.h"
#include "emi_dbg.h"


struct sk_dpr *emi_open(int addr_family){
    struct sk_dpr *sd;
    int proto;
    if((sd=(struct sk_dpr *)malloc(sizeof(struct sk_dpr)))==NULL)
        return NULL;
    switch(addr_family){
        case AF_INET:
            proto=0;
            break;
        default:
            proto=0;
    }
    if(AF_UNSPEC!=addr_family){
        if((sd->d=socket(addr_family,SOCK_STREAM,proto))<0){
            free(sd);
            return NULL;
        }
        sd->af=addr_family;
    }else{
    }
    return sd;
}

void emi_close(struct sk_dpr *sd){
    if(sd!=NULL){
        if(sd->d>0)
            close(sd->d);
        free(sd);
    }
}

int emi_connect(struct sk_dpr *sd,struct emi_addr *dest_addr,eu32 retry){
    unsigned int i;
    switch(sd->af){
        case AF_INET:
            for(i=1; i<=(2^retry); i<<=1){
                if(connect(sd->d, (const struct sockaddr *)&(dest_addr->ipv4), sizeof(struct sockaddr_in)) == 0)
                    return 0;
                if(i<(2^retry))
                    usleep(1000*(i<<3));
            }
            return -1;
        default:
            return -1;
    }
    return 0;
}

int emi_read(struct sk_dpr *sd,void *buf,eu32 size){
    int ret = 0;

    while((ret = read(sd->d,buf,size)) > 0){
        buf = (char *)buf + ret;
        size = size - ret;
    }
    return ret;
}

int emi_write(struct sk_dpr *sd,void *buf,eu32 size){
    int ret = 0;
    while((ret = write(sd->d,buf,size)) > 0){
        buf = (char *)buf + ret;
        size = size - ret;
    }
    return ret;
}

int emi_msg_write_payload(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_write(sd, (void *) msg, EMI_MSG_PAYLOAD_SIZE)) {
        return -1;
    }
    return 0;
}

static int emi_msg_write_data(struct sk_dpr *sd, struct emi_msg *msg){
    void *data = GET_ADDR(msg, msg->data_offset);
    if (emi_write(sd, data, msg->size)) {
        return -1;
    }
    return 0;
}

int emi_msg_write(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_msg_write_payload(sd, msg)){
        return -1;
    }

    if (msg->size > 0) {
        return emi_msg_write_data(sd, msg);    
    }
    return 0;
}

int emi_msg_read_payload(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_read(sd, msg, EMI_MSG_PAYLOAD_SIZE)) {
        return -1;
    }
    return 0;
}

static int emi_msg_read_retdata(struct sk_dpr *sd, struct emi_msg *msg){
    void *data = GET_ADDR(msg, msg->retdata_offset);
    if(emi_read(sd, data, msg->retsize)) {
        return -1;
    }
    return 0;
}

int emi_msg_read_ret(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_msg_read_payload(sd, msg)){
        return -1;
    }
    emilog(EMI_DEBUG, "Read with retdata, retsize %d\n", msg->retsize);

    if (msg->retsize > 0) {

        void *data = (char *)malloc(msg->retsize);
        if(data == NULL){
            return -1;
        }
        msg->retdata_offset = GET_OFFSET(msg, data);

        if (emi_msg_read_retdata(sd, msg)) {
            emi_msg_free_data(msg);
            return -1;
        }
    }

    return 0;
}
