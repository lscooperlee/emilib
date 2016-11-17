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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>
#include "emi.h"
#include "emi_sock.h"
#include "emi_config.h"
#include "emi_dbg.h"

#ifdef BLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#endif

struct emi_addr *emi_addr_alloc() {
    struct emi_addr *addr;
    if ((addr = (struct emi_addr *) malloc(sizeof(struct emi_addr))) == NULL) {
        goto error;
    }
    memset(addr, 0, sizeof(struct emi_addr));

    return addr;
error: 
    return NULL;
}

void emi_addr_free(struct emi_addr *addr) {
    free(addr);
}

struct emi_msg *emi_msg_alloc(eu32 size) {

    struct emi_msg *msg;
    if ((msg = (struct emi_msg *) malloc(sizeof(struct emi_msg) + size)) == NULL) {
        return NULL;
    }
    memset(msg, 0, sizeof(struct emi_msg) + size);
    msg->size = size;
    void *data = (char *)(msg + 1);
    msg->data_offset = GET_OFFSET(msg, data);

    return msg;
}

void emi_msg_free_data(struct emi_msg *msg) {
    if(msg->retsize > 0){
        void *data = GET_ADDR(msg, msg->retdata_offset);
        free(data);
        msg->retsize = 0;
    }
}

void emi_msg_free(struct emi_msg *msg) {
    emi_msg_free_data(msg);
    free(msg);
}

static int split_ipaddr(char *mixip, char *ip, int *port) {
    int ret = 0;
    char sip[32] = { 0 }, *p;
    strcpy(sip, mixip);
    p = strchr(sip, ':');
    if (p) {
        *p = '\0';
        *port = atoi(++p);
    } else {
        *port = USR_EMI_PORT;
        ret = -1;
    }
    strcpy(ip, sip);
    return ret;
}

int emi_fill_addr(struct emi_addr *addr, char *ip, int port) {

    if (((addr)->ipv4.sin_addr.s_addr = inet_addr(ip)) == -1)
        return -1;

    (addr)->ipv4.sin_port = htons(port);
    (addr)->ipv4.sin_family = AF_INET;
    (addr)->pid = getpid();
    return 0;
}

int emi_fill_msg(struct emi_msg *msg, char *dest_ip, void *data, eu32 cmd,
        eu32 defined_msg, eu32 flag) {
    if (dest_ip != NULL) {
        char newip[16];
        int port;
        split_ipaddr(dest_ip, newip, &port);
        if (emi_fill_addr(&(msg->addr), dest_ip, port))
            return -1;
    }

    if (data != NULL) {
        void *msgdata = GET_ADDR(msg, msg->data_offset);
        memcpy(msgdata, data, msg->size);
    }

    msg->cmd = cmd;
    msg->msg = defined_msg;
    msg->flag |= flag;

    return 0;
}

int emi_msg_send(struct emi_msg *msg) {
    struct sk_dpr *sd;
    int ret = -1;

    if ((sd = emi_open(AF_INET)) == NULL) {
        return -1;
    }
    if ((emi_connect(sd, &(msg->addr), 1)) < 0) {
        goto out;
    }

    if(emi_msg_write(sd, msg)){
        emilog(EMI_ERROR, "Error when writing msg, msg num %d\n", msg->msg);
        goto out;
    }

    emilog(EMI_DEBUG, "Msg sent succeeded, msg num %d\n", msg->msg);

    if (msg->flag & EMI_MSG_MODE_BLOCK) {
        if(emi_msg_read_ret(sd, msg)){
            emilog(EMI_ERROR, "Error when reading msg\n");
            goto out;
        }

        if(!(msg->flag&EMI_MSG_RET_SUCCEEDED)){
            emilog(EMI_DEBUG, "Block mode failed\n");
            ret = -1;
        }else{
            ret = 0;
        }

        emilog(EMI_DEBUG, "Ret data received, size %d\n", msg->retsize);
        debug_emi_msg(msg);

    } else {
        ret = 0;
    }


out: 
    emi_close(sd);
    return ret;
}

int emi_msg_send_highlevel(char *ipaddr, int msgnum, void *send_data,
        int send_size, void *ret_data, int ret_size, eu32 cmd, eu32 flag) {

    struct emi_msg *msg = emi_msg_alloc(send_size);
    if (msg == NULL) {
        return -1;
    }

    emi_fill_msg(msg, ipaddr, send_data, cmd, msgnum, flag);

    if (emi_msg_send(msg)) {
        emi_msg_free(msg);
        return -1;
    }

    if (ret_data != NULL && msg->retsize > 0) {
        void *data = GET_ADDR(msg, msg->retdata_offset);
        memcpy(ret_data, data, ret_size);
    }

    emi_msg_free(msg);
    return 0;
}

int emi_msg_send_highlevel_block(char *ipaddr, int msgnum, void *send_data,
        int send_size, void *ret_data, int ret_size, eu32 cmd) {
    eu32 flag = EMI_MSG_MODE_BLOCK;

    return emi_msg_send_highlevel(ipaddr, msgnum, send_data, send_size,
            ret_data, ret_size, cmd, flag);
}

int emi_msg_send_highlevel_nonblock(char *ipaddr, int msgnum, void *send_data,
        int send_size, eu32 cmd) {
    eu32 flag = 0;

    return emi_msg_send_highlevel(ipaddr, msgnum, send_data, send_size, NULL, 0,
            cmd, flag);
}
