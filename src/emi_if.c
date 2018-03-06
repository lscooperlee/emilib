#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "emi_if.h"
#include "emi_msg.h"
#include "emi_sock.h"
#include "emi_dbg.h"
#include "emi_config.h"

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
        free(get_next_retdata(msg, NULL));
        msg->retsize = 0;
    }
}

void emi_msg_free(struct emi_msg *msg) {
    emi_msg_free_data(msg);
    free(msg);
}

static int split_ipaddr(const char *mixip, char *ip, int *port) {
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

static int emi_addr_init(struct emi_addr *addr, const char *ip, int port) {

    if ((addr->ipv4.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
        return -1;

    addr->ipv4.sin_port = htons(port);
    addr->ipv4.sin_family = AF_INET;
    addr->pid = getpid();
    return 0;
}

int emi_msg_init(struct emi_msg *msg, const char *dest_ip, const void *data, eu32 cmd,
        eu32 defined_msg, eu32 flag) {
    if (dest_ip != NULL) {
        char newip[16];
        int port;
        split_ipaddr(dest_ip, newip, &port);
        if (emi_addr_init(&(msg->addr), dest_ip, port))
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

    emilog(EMI_DEBUG, "Start sending msg");

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

    } else {
        ret = 0;
    }


out: 
    emi_close(sd);
    return ret;
}
