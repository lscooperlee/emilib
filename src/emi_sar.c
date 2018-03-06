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

char retdata[1024] = {0};
unsigned int retsize = 0;

unsigned int sentsize=0;
char sentdata[1024]={0};

void print_msg(struct emi_msg const *msg){
    printf("msg = %X, ",msg->msg);
    printf("cmd = %X, ",msg->cmd);
    printf("msg.size = %ud",msg->size);
    if(msg->size > 0){
        char *data = GET_DATA(msg);
        printf(", msg.data = %s\n", data);
    }else{
        printf("\n");
    }
}

void print_data(unsigned int size, const char *data){
    if(size > 0){
        char *tmp = (char *)malloc(size + 1);
        if(tmp == NULL)
            return;

        strncpy(tmp, data, size);
        *(tmp + size) = '\0';
        printf("retdata = %s\n", tmp);

        free(tmp);
    }
}

int func_noblock(struct emi_msg const *msg){
    print_msg(msg);
    return 0;
}

int func_block(struct emi_msg const *msg){
    print_msg(msg);
    print_data(retsize, retdata);
    return emi_load_retdata(msg, retdata, retsize); 
}

void usage(void){
    printf("usage: sar -r msg [-R retdata]\n");
    printf("usage: sar [-b] -s addr -m msg [ -c cmd ] [-d sentdata]\n");
}

#define BLOCK_MODE     0x20
#define REGISTER_MSG 0x1
#define SEND_MSG 0x2
#define MSG_NUM 0x10
#define MSG_CMD 0x8
#define MSG_DATA 0x4
#define MSG_RETDATA 0x40

int main(int argc,char **argv){

    char opt;
    int option=0;
    long msgr=-1,msgnum=-1;
    unsigned long cmd=0;
    char addr[32]={0};

    if(argc==1){
        usage();
        return 0;
    }

    while(*++argv!=NULL){
        while(**argv=='-'){
            while((opt=*++*argv)!='\0'){
                switch(opt){
                    case 'b':
                        option|=BLOCK_MODE;         //for block mode
                        break;
                    case 'r':
                        msgr=atol(*(argv+1));
                        if(msgr>=0)
                            option|=REGISTER_MSG;     //for register
                        break;
                    case 's':
                        strcpy(addr,*(argv+1));
                        if (*(char *)addr != 0)
                            option|=SEND_MSG;      //for send msg
                        break;
                    case 'd':
                        option|=MSG_DATA;     //for send data
                        sentsize=strlen(*(argv+1));
                        strcpy(sentdata,*(argv+1));
                        break;
                    case 'R':
                        option|=MSG_DATA;     //ret data of the sender
                        retsize=strlen(*(argv+1));
                        strcpy(retdata,*(argv+1));
                        break;
                    case 'c':             //for send cmd
                        option|=MSG_CMD;
                        cmd=atol(*(argv+1));
                        break;
                    case 'm':             //for send msg
                        msgnum=atol(*(argv+1));
                        if (msgnum >=0 )
                            option|=MSG_NUM;
                        break;
                    default:
                        option=-1;
                }
                break;
            }
        }

    }

    setbuf(stdout, NULL);

    if(option&REGISTER_MSG){
        if(emi_init()){
            printf("emi_init error\n");
            return -1;
        }
        
        emi_func f;

        if(option&MSG_DATA){
            f = func_block;

        }else{
            f = func_noblock;
        }

        if(emi_msg_register(msgr,f)){
            printf("emi_msg_register error\n");
            return -1;
        }

        emi_loop();

    }else if(option&(SEND_MSG|MSG_NUM)){

        struct emi_msg *msg = emi_msg_alloc(sentsize);
        if (msg == NULL) {
            return -1;
        }

        emi_msg_init(msg, addr, sentdata, cmd, msgnum, 0);

        if(option&BLOCK_MODE){
            msg->flag |= EMI_MSG_MODE_BLOCK;

            print_msg(msg);            

            if (emi_msg_send(msg)) {
                printf("block msg send failed\n");
                emi_msg_free(msg);
                return -1;
            }
            
            struct emi_retdata *retdata;
            for_each_retdata(msg, retdata){
                print_data(retdata->size, retdata->data);
            }

        }else{

            print_msg(msg);            

            if (emi_msg_send(msg)) {
                printf("nonblock msg send failed\n");
                emi_msg_free(msg);
                return -1;
            }

        }

        emi_msg_free(msg);
    }else{
        usage();
    }

    return 0;
}
