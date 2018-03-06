
#include <iostream>
#include <string>

#include "emi.h"


int main(int argc, char **argv){

    std::string data;
    if(argc == 2){
        data.assign(argv[1]);
    }

    auto msg = make_emi_msg("127.0.0.1", 1, 2, data, EMI_MSG_MODE_BLOCK);
    auto ret = emi_msg_send(msg);

    std::cout<<ret<<std::endl;
    std::cout<<msg->retsize<<std::endl;
    
    struct emi_retdata *retdata;
    for_each_retdata(msg, retdata){
//    for(auto retdata: msg->retdata()){
        std::cout<<std::string((char *)retdata->data, msg->size)<<std::endl;
    }
}
