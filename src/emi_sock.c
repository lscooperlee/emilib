/*
EMI:    embedded message interface
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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "emi.h"
#include "emi_sock.h"


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
int emi_listen(struct sk_dpr *sd){
    if(listen(sd->d,SOMAXCONN)<0){
        return -1;
    }
    return 0;
}

int emi_connect(struct sk_dpr *sd,struct emi_addr *dest_addr,eu32 retry){
    int i,ret;
    switch(sd->af){
        case AF_INET:
            for(i=1;i<=(2^retry);i<<=1){
                if((ret=connect(sd->d,(const struct sockaddr *)&(dest_addr->ipv4),sizeof(struct sockaddr_in)))==0)
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

int emi_read(struct sk_dpr *sd,void *buf,eu32 size){
    return read(sd->d,buf,size);
}

int emi_write(struct sk_dpr *sd,void *buf,eu32 size){
    return write(sd->d,buf,size);
}

struct sk_dpr *emi_accept(struct sk_dpr *sd,union emi_sock_addr *addr){
    struct sk_dpr *sd_dest;
    socklen_t len;
    len=sizeof(union emi_sock_addr);
    if((sd_dest=(struct sk_dpr *)malloc(sizeof(struct sk_dpr)))==NULL)
        return NULL;
    if((sd_dest->d=accept(sd->d,(struct sockaddr *)addr,&len))<0){
        return NULL;
    }
    if(addr!=NULL){
        sd_dest->af=((struct sockaddr *)addr)->sa_family;
    }else{
        sd_dest->af=0;
    }
    return sd_dest;
}

int emi_msg_write_payload(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_write(sd, (void *) msg, EMI_MSG_PAYLOAD_SIZE) < EMI_MSG_PAYLOAD_SIZE) {
        return -1;
    }
    return 0;
}

int emi_msg_write_data(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_write(sd, msg->data, msg->size) < msg->size) {
        return -1;
    }
    return 0;
}

int emi_msg_write(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_msg_write_payload(sd, msg)){
        return -1;
    }
    if (msg->size > 0 && (void *) msg->data != NULL) {
        return emi_msg_write_data(sd, msg);    
    }
    return 0;
}


int emi_msg_read_payload(struct sk_dpr *sd, struct emi_msg *msg){
    if (emi_read(sd, msg, EMI_MSG_PAYLOAD_SIZE) < EMI_MSG_PAYLOAD_SIZE) {
        return -1;
    }
    return 0;
}

int emi_msg_read_data(struct sk_dpr *sd, struct emi_msg *msg){
    if(emi_read(sd, msg->data, msg->size) < msg->size) {
        return -1;
    }
    return 0;
}

int emi_msg_read(struct sk_dpr *sd, struct emi_msg *msg){
    unsigned int oldsize = msg->size;
    if (emi_msg_read_payload(sd, msg)){
        return -1;
    }

    if (msg->size > 0) {
        if(msg->size > oldsize){
            msg->data = (char *)malloc(msg->size);
            if(msg->data == NULL){
                return -1;
            }
            msg->flag |= EMI_MSG_FLAG_ALLOCDATA;
        }

        if (emi_msg_read_data(sd, msg)) {
            return -1;
        }
    }

    return 0;
}
