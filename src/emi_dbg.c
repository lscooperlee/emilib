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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "emi.h"
#include "emi_shbuf.h"
#include "emi_dbg.h"


void print_emi_msg(struct emi_msg *msg, void *buf){
    char *data = GET_ADDR(msg, msg->data_offset);
    sprintf(buf, "emi_msg: msg=%d, cmd=%d, size=%d, offset = %lx, ret_offset = %lx ", msg->msg, msg->cmd, msg->size, msg->data_offset, msg->retdata_offset);

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

void print_emi_buf(struct emi_buf *emibuf, void *buf){
    sprintf(buf, "emi_shbuf: blk_offset=%d, order=%d ", emibuf->blk_offset, emibuf->order);
}

void debug_emi_msg(struct emi_msg *msg){
    char buf[4096];
    print_emi_msg(msg, buf);
    syslog(EMI_DEBUG, "%s", buf);
}

