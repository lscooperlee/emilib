
#include <iostream>
#include <vector>
#include <algorithm>

#include "emi.h"


int main(void){
    
    auto recv = [](struct emi_msg const *msg){
        std::string data((char *)GET_DATA(msg), (char *)GET_DATA(msg) + msg->size);

        std::cout<<msg->msg<<" "<<msg->cmd<<" "<<data<<std::endl;

        if (msg->size > 0){
            std::reverse(data.begin(), data.end());
            return emi_load_retdata(msg, data);
        }

        return 0;
    };

    emi_init();

    emi_msg_register(1, recv);

    emi_loop();
}
