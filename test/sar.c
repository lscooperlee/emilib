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
#include <emiif.h>
#include <emi_dbg.h>


int func(struct emi_msg *tmp){
    return 0;
}

void usage(void){
    printf("usage:sar [-b] -r registermsg \n usage: sar [-b] -s addr -m msg [ -c cmd ] [-d data]\n");
}

#define BLOCK_MODE     0x20
#define REGISTER_MSG 0x1
#define SEND_MSG 0x2
#define MSG_NUM 0x10
#define MSG_CMD 0x8
#define MSG_DATA 0x4

int main(int argc,char **argv){

    char opt;
    int option=0;
    int size=0;
    char datap[1024]={0};
    char retdatap[1024]={0};

    unsigned long cmd=0,msgr=-1,msgnum=-1;
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
                        if (*(int *)addr != 0)
                            option|=SEND_MSG;      //for send msg
                        break;
                    case 'd':
                        option|=MSG_DATA;     //for send data
                        size=strlen(*(argv+1));
                        strcpy(datap,*(argv+1));
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


    if(option&REGISTER_MSG){
        if(emi_init()){
            printf("emi_init error\n");
        }
        emi_msg_register(msgr,func);
        emi_loop();

    }else if(option&(SEND_MSG|MSG_NUM)){
        if(option&BLOCK_MODE){
            emi_msg_send_highlevel_block(addr,msgnum,datap,size, retdatap, 1024, cmd);
            printf("%s\n",retdatap);
        }else{
            emi_msg_send_highlevel_nonblock(addr,msgnum,datap,size, cmd);
        }
    }else{
        usage();
    }


}
