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




char retdata[30]="hello world";


int func_nonblock_no_data1(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}

int func_nonblock_with_data1(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}

int func_nonblock_no_data2(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}

int func_nonblock_with_data2(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}


int func_block_no_sendata_no_retdata(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}

int func_block_with_sendata_no_retdata(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return 0;
}

int func_block_no_sendata_with_retdata(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return emi_msg_prepare_return_data(msg,retdata,sizeof(retdata));
}

int func_block_with_sendata_with_retdata(struct emi_msg *msg){
    sleep(1);
    debug_msg_body(msg);
    printf("%s\n\n\n",__func__);
    return emi_msg_prepare_return_data(msg,retdata,sizeof(retdata));
}


int main(int argc,char **argv){

    int ret;
    ret=emi_init();
    printf("emi_init ret=%d\n",ret);


    ret=emi_msg_register(TEST_NONBLOCK_NO_SENDATA,func_nonblock_no_data1);
    printf("emi_msg_register :TEST_NONBLOCK_NO_SENDATA ret=%d\n",ret);
    ret=emi_msg_register(TEST_NONBLOCK_NO_SENDATA,func_nonblock_no_data2);
    printf("emi_msg_register :TEST_NONBLOCK_NO_SENDATA ret=%d\n",ret);

    ret=emi_msg_register(TEST_NONBLOCK_WITH_SENDATA,func_nonblock_with_data1);
    printf("emi_msg_register :TEST_NONBLOCK_WITH_SENDATA ret=%d\n",ret);
    ret=emi_msg_register(TEST_NONBLOCK_WITH_SENDATA,func_nonblock_with_data2);
    printf("emi_msg_register :TEST_NONBLOCK_WITH_SENDATA ret=%d\n",ret);


    ret=emi_msg_register(TEST_BLOCK_NO_SENDATA_NO_RETDATA,func_block_no_sendata_no_retdata);
    printf("emi_msg_register :TEST_BLOCK_NO_SENDATA_NO_RETDATA ret=%d\n",ret);
    ret=emi_msg_register(TEST_BLOCK_NO_SENDATA_WITH_RETDATA,func_block_no_sendata_with_retdata);
    printf("emi_msg_register :TEST_BLOCK_NO_SENDATA_WITH_RETDATA ret=%d\n",ret);

    ret=emi_msg_register(TEST_BLOCK_WITH_SENDATA_NO_RETDATA,func_block_with_sendata_no_retdata);
    printf("emi_msg_register :TEST_BLOCK_WITH_SENDATA_NO_RETDATA ret=%d\n",ret);
    ret=emi_msg_register(TEST_BLOCK_WITH_SENDATA_WITH_RETDATA,func_block_with_sendata_with_retdata);
    printf("emi_msg_register :TEST_BLOCK_WITH_SENDATA_WITH_RETDATA ret=%d\n",ret);

    emi_loop();
}
