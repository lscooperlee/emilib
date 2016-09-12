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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "emi.h"
#include "emi_config.h"
#include "msg_table.h"


struct msgandnum{
    char msg[128];
    eu32 num;
};

struct msgandnum msgtonum[]={
    {
        .msg="EMI_MSG_MODE_BLOCK",
        .num=0x00000100,
    },
    {
        .msg="EMI_MSG_CMD_REGISTER",
        .num=0x00004000,
    },
    {
        .msg="EMI_MSG_RET_SUCCEEDED",
        .num=0x00010000,
    },
};

#define ARRAY_SIZE(array)    (sizeof(array)/sizeof(array[0]))

void debug_flag(eu32 flag){
    int i,j;
    for(i=0,j=0;i<ARRAY_SIZE(msgtonum);i++){
        if(flag&(msgtonum[i].num)){
            j++;
            printf("        flag include %s,%X\n",msgtonum[i].msg,msgtonum[i].num);
        }
    }
    if(!j)
        printf("no flag matched\n");

}


void debug_addr(struct emi_addr *addr,char *p){
    printf("    %s debuging:    ",p);
    printf("address=%lX, ",(long)addr);
    printf("ip=%s, ",inet_ntoa(addr->ipv4.sin_addr));
    printf("port=%d, ",ntohs(addr->ipv4.sin_port));
    printf("pid=%d\n",addr->pid);

}

void debug_msg_body(struct emi_msg *msg){
    printf("msg->size is %d\n",msg->size);
    printf("msg->data address is %lX\n", (long)(msg->data));
    if(msg->size){
        printf(" date size is %d,content is %s:\n",msg->size,msg->data);
        int i;

        for(i=0;i<msg->size;i++){
            printf("%d ",msg->data[i]);
        }
    }
    printf(" emi_msg cmd is %X\n",msg->cmd);
    printf(" msg is %X\n",msg->msg);
    printf(" flag=%X\n",msg->flag);
    return;
}

void debug_msg(struct emi_msg *msg,int more){
    printf("emi_msg debuging\n");
    printf(" emi_msg address is %lX\n",(long)msg);

    if(!more)
        return;

    debug_addr(&msg->src_addr,"src_addr");
//    printf("%s\n",inet_ntoa((msg->dest_addr->ipv4.sin_addr)));
//    if(msg->dest_addr!=NULL)
//        debug_addr(msg->dest_addr,"dest_addr");

    debug_msg_body(msg);

    debug_flag(msg->flag);
    return;
}

void debug_single_map(struct msg_map *map){
    if(map!=NULL)
        printf("    single_map debuging:msg=%X,pid=%d,next=%lX\n",map->msg,map->pid,(long)map->next);
    else
        printf("    single_map is NULL\n");
    return;
}
void debug_msg_map(struct msg_map **table,struct msg_map *map){
    eu32 i;
    struct msg_map *tmp;
    printf("msg_map debugging:\n");
    i=emi_hash(map);
    printf("    hash value is %d\n",i);
    if((tmp=emi_hsearch(table,map))==NULL){
        printf("    map is not in the table\n");
    }else{
        debug_single_map(tmp);
    }
}

void debug_msg_chain(struct msg_map **table,struct msg_map *map){
    printf("\n");
    int j;
    struct msg_map *tmp;
    printf("msg_chain debugging:\n");
    for(j=0,tmp=(*(table+emi_hash(map)));tmp!=NULL;tmp=tmp->next,j++){
        printf("    chan %d: ",j);
        debug_single_map(tmp);
    }
    printf("\n");
}

void debug_msg_table(struct msg_map **table){
    int i,j;
    struct msg_map *tmp;

    printf("printing msg_table\n");
    for(i=0;i<EMI_MSG_TABLE_SIZE;i++){
            printf("  line %d",i);
            for(j=0,tmp=(*(table+i));tmp!=NULL;tmp=tmp->next,j++){
                printf("\n        chan %d: ",j);
                debug_single_map(tmp);
            }
            printf("\n");
    }
    return;
}

void debug_msg_full_table(struct msg_map **table){
    int i,j;
    struct msg_map *tmp;

    printf("printing full msg_table\n");
    for(i=0;i<EMI_MSG_TABLE_SIZE;i++){
        if(*(table+i)!=NULL){
            printf("  line %d",i);
            for(j=0,tmp=(*(table+i));tmp!=NULL;tmp=tmp->next,j++){
                printf("\n        chan %d: ",j);
                debug_single_map(tmp);
            }
            printf("\n");
        }else{
            continue;
        }
    }
    return;
}

