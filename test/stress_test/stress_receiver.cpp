
#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>

#include "emi.h"

void block_receive(int times) {
    auto recv = [](struct emi_msg const *msg){
        std::string data((char *)GET_DATA(msg), (char *)GET_DATA(msg) + msg->size);

        if (msg->size > 0){
            std::reverse(data.begin(), data.end());
            return emi_load_retdata(msg, data);
        }

        return 0;
    };

    for(int i = 0; i < times; ++i) {
        auto ret = emi_msg_register(i, recv);
        if(ret < 0) {
            std::cout<<"stress test failed\n"<<std::endl;
        }
    }
}

int main(void){
    
    emi_init();

    block_receive(100);
    
    while(1)
        pause();
}
