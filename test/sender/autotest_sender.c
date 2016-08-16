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
#include "autotest.h"
int main(int argc,char **argv){

    if(argc!=2){
        printf("usage: %s ip_addr\n",argv[0]);
        return 0;
    }

    char *ip=argv[1];
    int ret;
    char sendata[]={'h','e','l','l','o','\0'};
    char retdata[sizeof(sendata)*200]={0};

    while(1){
        printf("in process\n");

        ret=emi_msg_send_highlevel_nonblock(ip,TEST_NONBLOCK_NO_SENDATA,NULL, 0, 1);
        printf("test TEST_NONBLOCK_NO_SENDATA ret=%d\n",ret);

        ret=emi_msg_send_highlevel_nonblock(ip,TEST_NONBLOCK_WITH_SENDATA,sendata, sizeof(sendata), 2);
        printf("test TEST_NONBLOCK_WITH_SENDATA ret=%d\n",ret);

        ret=emi_msg_send_highlevel_block(ip,TEST_BLOCK_NO_SENDATA_NO_RETDATA,NULL, 0, NULL, 0, 3);
        printf("test TEST_BLOCK_NO_SENDATA_NO_RETDATA ret=%d\n",ret);

        ret=emi_msg_send_highlevel_block(ip,TEST_BLOCK_WITH_SENDATA_NO_RETDATA,sendata, sizeof(sendata), NULL, 0, 4);
        printf("test TEST_BLOCK_WITH_SENDATA_NO_RETDATA ret=%d\n",ret);

        ret=emi_msg_send_highlevel_block(ip,TEST_BLOCK_NO_SENDATA_WITH_RETDATA,NULL, 0, retdata, sizeof(retdata), 5);
        printf("test TEST_BLOCK_NO_SENDATA_WITH_RETDATA ret=%d,retdata=%s\n",ret,retdata);

        ret=emi_msg_send_highlevel_block(ip,TEST_BLOCK_WITH_SENDATA_WITH_RETDATA,sendata, sizeof(sendata), retdata, sizeof(retdata), 6);
        printf("test TEST_BLOCK_WITH_SENDATA_WITH_RETDATA ret=%d,retdata=%s\n",ret,retdata);
        getchar();

    }

    return 0;
}
