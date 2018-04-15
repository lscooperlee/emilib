#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "emi_msg.h"
#include "emi_dbg.h"

/*
void print_emi_msg(struct emi_msg const *msg, char *buf){
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
*/

void _emilog(int priority, char *buf, const char *format, ...){
    if(priority <= DEDAULT_LOGLEVEL) {
        va_list list; 
        va_start(list, format); 
        vsprintf(buf+strlen(buf), format, list);
        va_end(list);
        if(buf[strlen(buf) - 1] != '\n')
            fprintf(stderr, "%s\n", buf);
        else
            fprintf(stderr, "%s", buf);
    }
}
