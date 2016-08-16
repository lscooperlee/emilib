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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include "emi.h"
#include "emisocket.h"
#include "emi_config.h"
#include "emi_dbg.h"


#ifdef BLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#endif

struct emi_addr *emi_addr_alloc(){
    struct emi_addr *addr;
    if((addr=(struct emi_addr *)malloc(sizeof(struct emi_addr)))==NULL){
        goto error;
    }
    memset(addr,0,sizeof(struct emi_addr));

    return addr;
error:
    return NULL;
}

void emi_addr_free(struct emi_addr *addr){

    free(addr);
    return;
}

struct emi_msg *emi_msg_alloc(eu32 size){

    struct emi_msg *msg;
    if((msg=(struct emi_msg *)malloc(sizeof(struct emi_msg)+size))==NULL){
        goto error;
    }
    memset(msg,0,sizeof(struct emi_msg));
    msg->size=size;

    return msg;

error:
    return NULL;
}

void emi_msg_free(struct emi_msg *msg){
    free(msg);
    return;
}

static int split_ipaddr(char *mixip,char *ip,int *port){
    int ret=0;
    char sip[32]={0},*p;
    strcpy(sip,mixip);
    p=strchr(sip,':');
    if(p){
        *p='\0';
        *port=atoi(++p);
    }else{
        *port=USR_EMI_PORT;
        ret=-1;
    }
    strcpy(ip,sip);
    return ret;
}

int emi_fill_addr(struct emi_addr *addr,char *ip,int port){

    if(((addr)->ipv4.sin_addr.s_addr=inet_addr(ip))==-1)
        return -1;

    (addr)->ipv4.sin_port=htons(port);
    (addr)->ipv4.sin_family=AF_INET;
    (addr)->pid=getpid();
    return 0;
}

int emi_fill_msg(struct emi_msg *msg,char *dest_ip,void *data,eu32 cmd,eu32 defined_msg,eu32 flag){
    struct emi_addr *p=&(msg)->src_addr;
    if(emi_fill_addr(p,"127.0.0.1",emi_config->emi_port))
        return -1;
    if(dest_ip!=NULL){
        char newip[16];
        int port;
        split_ipaddr(dest_ip,newip,&port);
        if(emi_fill_addr(&(msg->dest_addr),dest_ip,port))
            return -2;
    }
    if(data!=NULL){
//the massage could be zero,do not deem it as the end of the massage.
//        eu32 n=strlen(data);
//        n=(n<msg->size)?n:msg->size;
        memcpy((msg)->data,data,msg->size);
    }
    (msg)->cmd=cmd;
    (msg)->msg=defined_msg;
    (msg)->flag|=flag;
    return 0;
}

int emi_msg_send(struct emi_msg *msg){
    struct sk_dpr *sd;
    int ret=-1;

    if((sd=emi_open(AF_INET))==NULL){
        dbg("emi_open error\n");
        return -1;
    }
    if((emi_connect(sd,&(msg->dest_addr),1))<0){
        dbg("remote connected error\n");
        goto out;
    }

    if((emi_write(sd,(void *)msg,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
        dbg("block mode:error when emi_write to remote daemon\n");
        goto out;
    }

    if(msg->size > 0 && msg->data != NULL){
        if((emi_write(sd,msg->data,msg->size))<msg->size){
            dbg("nonblock mode:DATA:emi_write to remote prcess local data with error\n");
            goto out;
        }
    }

        //waiting for optimizing
    if(msg->flag&EMI_MSG_MODE_BLOCK){

/*
 * keep consistent with the emi_core, try to read an emi_msg struct first, if the msg handler function returns an error, emi_core 
 *      will not send us the emi_msg struct, but we don't know that, so try to read it, if we read returns an error, that means
 *      emi_core has close the client_sd (accepted socket), some errors must be occured.
 *
 *         there are two things that should be noticed.
 *
 *         first, currently
 *         the returned extra data will be not larger than the sent msg->size data, because we have no way to return this
 *         extra data ,so we will use msg->data to store the returned extra data, which means the returned data size should not
 *         large than the sent data size.
 *
 *         NOTE: THAT this will probably be changed in the future, if we use buddy allocator to manager the msg->data space,
 *         than we can return even bigger size of extra data back.
 *
 *
 *         second, currently
 *         this returned extra data is not required to be check and it will be probably received with no error,
 *         if we receive returned emi_msg, it must be a SCCEEDED state, we don't have to check SCCEEDED again for
 *         receiving extra data if it has.
 *         because all the  checks were done by the emi_msg_prepare_return and func_sterotype, see the two functions for details.
 *
 */
        if((emi_read(sd,msg,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
            dbg("block mode:read msg emi_read from remote process error\n");
            goto out;
        }
        if(msg->size > 0){
            if ((emi_read(sd, msg->data, msg->size)) < msg->size) {
                dbg("block mode:read extra data emi_read from remote process error\n");
                goto out;
            }
        }
        ret=0;
        dbg("a block msg sent successfully\n");
    }else{
        ret=0;
        dbg("a nonblock msg sent successfully\n");
    }
out:
    emi_close(sd);
    return ret;
}

static inline int emi_msg_send_highlevel(char *ipaddr, int msgnum,void *send_data,int send_size, void *ret_data,int ret_size, eu32 cmd,eu32 flag){

    /* make sure the ret data size is no lager than send data size,
     * currently, the ret data is using the memory of the send data to receive
     * the ret data.
     */
    int size = send_size > ret_size ? send_size : ret_size;

    struct emi_msg *msg = emi_msg_alloc(size);
    if (msg == NULL) {
        dbg("emi_msg_alloc error\n");
        return -1;
    }

    emi_fill_msg(msg, ipaddr, send_data, cmd, msgnum, flag);
    if (emi_msg_send(msg)) {
        dbg("emi_msg_send error\n");
        emi_msg_free(msg);
        return -1;
    }

    if (ret_data != NULL && msg->size > 0) {
        memcpy(ret_data, msg->data, msg->size);
    }

    emi_msg_free(msg);
    return 0;
}

/*
 * NOTE:
 *     BE CAREFUL, the ret_size in this function is supposed to be the expected return data size from the receiver.
 *         normally this should be awared by both receiver and sender.
 *         the receiver can return data as much as possible, only limited by the emi max space range.
 *         but the sender can not know how much data the receiver will return, if the receiver returns a large amount of data,
 *         but the sender only prepares a small size buffer to store it, the sender will probably get some kind of system memory error.
 *
 *         this of course can be fixed by cycling read and realloc, but I don't want to do that recently.
 *
 */
int emi_msg_send_highlevel_block(char *ipaddr, int msgnum,void *send_data,int send_size,void *ret_data,  int ret_size,eu32 cmd){
    eu32 flag=EMI_MSG_MODE_BLOCK;

    return emi_msg_send_highlevel(ipaddr,msgnum,send_data,send_size,ret_data,ret_size,cmd,flag);
}

int emi_msg_send_highlevel_nonblock(char *ipaddr, int msgnum,void *send_data, int send_size, eu32 cmd){
    eu32 flag=0;

    return emi_msg_send_highlevel(ipaddr,msgnum,send_data,send_size,NULL,0,cmd,flag);
}

/*
 * this function should be called inside msg handler function, preparing returned extra data, several steps:
 *      step 1: check if size > MAX_SIZE_THAT_CAN_BE_ALLOC, at this version emi_config->emi_data_size_per_msg,
 *      the ret data size can be larger than the received data size, an example is the sender has no data to be transmit to
 *      the receiver, thus the received data size should be zero, but need the receiver to write back an extra data, so the
 *      returned data size is larger than the received one.
 *
 *      step 2: check if msg is ~BLOCK. Because the sender will not receive returned msg
 *      if it sends an ~BLOCK msg.
 *
 *      step 3: change msg->flag to EMI_MSG_RET_SUCCEEDED, see also func_sterotype
 *
 *      when using this function in msg handler,
 *      we must check the return value of this function and must not return 0(success)
 *      in msg handler when emi_msg_prepare_return_data return -1(fail)
 */
int emi_msg_prepare_return_data(struct emi_msg *msg, void *data, eu32 size) {

    if (size > emi_config->emi_data_size_per_msg) {
        dbg("the size of returned extra data is too large\n");
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        return -1;
    }

    if (!(msg->flag & EMI_MSG_MODE_BLOCK)) {
        dbg("an ~BLOCK msg is sent, receiver is not expecting receive data");
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        return -1;
    }

    msg->size = size;
    memcpy(msg->data, data, size);
    msg->flag |= EMI_MSG_RET_SUCCEEDED;
    return 0;
}

/**
 *Note: these functions are used for transporting emilib into other programming language like Python, it is more convienent to just 
     use basic types when transporting (int, char and all kinds of pointers ) rather than structure. many transporting methods may
    be hard to deal with complicated C types. these functions are much useful and can make things easer.

    of course you can use them in C too.
 *
 */
eu32 get_msgnum_from_msg(struct emi_msg *msg){
    return msg->msg;
}
eu32 get_cmd_from_msg(struct emi_msg *msg){
    return msg->cmd;
}

char *get_data_from_msg(struct emi_msg *msg){
    return msg->data;
}
eu32 get_datasize_from_msg(struct emi_msg *msg){
    return msg->size;
}

void copy_data_from_msg(struct emi_msg *msg,void *dest){
    memcpy(dest,msg->data,msg->size);
}
