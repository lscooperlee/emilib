#include <stdio.h>

#include "emi_if.h"


#define TEST_MSG 0x1

int func(struct emi_msg *msg){
    printf("cmd: %d\n",msg->cmd);
    printf("data: %s\n",msg->data);
    return 0;
}

int main(){
    emi_init();

    emi_msg_register(TEST_MSG, func);

    emi_msg_send_highlevel("127.0.0.1", TEST_MSG, "hello world!",
        sizeof("hello world!"), NULL, 0, 1, 0);

    sleep(1);
}