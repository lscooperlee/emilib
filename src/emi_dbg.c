#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "emi_msg.h"
#include "emi_dbg.h"


void print_emi_msg(struct emi_msg *msg, char *buf){
    char *data = GET_ADDR(msg, msg->data_offset);
    sprintf(buf, "emi_msg: msg=%ud, cmd=%ud, size=%ud, offset = %lx, ret_offset = %lx ", 
            msg->msg, msg->cmd, msg->size, msg->data_offset, msg->retdata_offset);

    if(data != NULL && msg->size != 0){
        int len = strlen(buf);
        char *tmp = buf+len;
        sprintf(tmp, ", data: ");
        tmp = strlen(buf) + buf;

        int i, datasize = msg->size > 8 ? 8 : msg->size;
        for(i=0; i<datasize; i++){
            int t = data[i] & 0xff;
            sprintf(tmp + i*3, "%02x ", t);

        }
    }


    char *retdata = GET_ADDR(msg, msg->retdata_offset);
    if(retdata != NULL && msg->retsize != 0){
        int len = strlen(buf);
        char *tmp = buf+len;

        sprintf(tmp, ", retdata: ");
        tmp = strlen(buf) + buf;

        int i, datasize = msg->retsize > 8 ? 8 : msg->retsize;
        for(i=0; i<datasize; i++){
            int t = retdata[i] & 0xff;
            sprintf(tmp + i*3, "%02x ", t);

        }
    }

}


void debug_emi_msg(struct emi_msg *msg){
    char buf[4096];
    print_emi_msg(msg, buf);
    syslog(EMI_DEBUG, "%s", buf);
}

