#include <string>
#include <thread>
#include <vector>
#include <array>
#include <unistd.h>

#include "emi.h"

#include <iostream>
void unblock_send(int times) {
    std::array<char, 16> data={};
    std::vector<std::thread> workers;
    std::vector<emi_msg_ptr> msgs;

    for(int i = 0; i < times; ++i) {
        auto msg=make_emi_msg("127.0.0.1", i, i*2, data, EMI_MSG_MODE_BLOCK);
        msgs.push_back(std::move(msg));
    }

    for(int i = 0; i < times; ++i) {
        workers.push_back(std::thread([m=std::move(msgs[i])] { 
            if(m){
                auto ret = emi_msg_send(m);
                // for(auto retdata: m->retdata()){ }

                if(ret) {
                    std::cout<<"stress test failed"<<std::endl;
                }
            }
        }));
    }

    for (auto& worker : workers) {
        worker.join();
    }
}


int main(void) {
    unblock_send(100);
}
