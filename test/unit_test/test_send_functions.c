#include <stdio.h>

#include "emi.h"


#define TEST_MSG 0x1

int func(struct emi_msg const *msg){
    printf("cmd: %d\n",msg->cmd);
    char *data = GET_ADDR(msg, msg->data_offset);
    printf("data: %s\n", data);
    return 0;
}

int main(){
    emi_init();

    emi_msg_register(TEST_MSG, func);

    emi_msg_send_highlevel("127.0.0.1", TEST_MSG, "hello world!",
        sizeof("hello world!"), NULL, 0, 1, 0);

    sleep(1);
}
