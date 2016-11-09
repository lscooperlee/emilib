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
#include "emi_dbg.h"


void print_emi_msg(struct emi_msg *msg, void *buf){
    sprintf(buf, "emi_msg: msg=%d, cmd=%d, size=%d, offset = %ld ", msg->msg, msg->cmd, msg->size, msg->data_offset);
    char *data = GET_ADDR(msg, msg->data_offset);
    if(data != NULL && msg->size != 0){
        int len = strlen(buf);
        char *tmp = buf+len;

        int i, datasize = msg->size > 8 ? 8 : msg->size;
        for(i=0; i<datasize; i++){
            int t = data[i] & 0xff;
            sprintf(tmp + i*3, "%02x ", t);

        }
    }
}

void debug_emi_msg(struct emi_msg *msg){
    char buf[1024];
    print_emi_msg(msg, buf);
    syslog(EMI_DEBUG, "%s", buf);
}

