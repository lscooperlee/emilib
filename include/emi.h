
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "emi_msg.h"
#include "emi_if.h"

#ifndef SEND_ONLY
#include "emi_ifr.h"
#endif



#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
#include <memory>
#include <functional>

using emi_msg_ptr = std::unique_ptr<emi_msg, std::function<decltype(emi_msg_free)>>;
inline emi_msg_ptr make_emi_msg_ptr(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        eu32 data_size, const void *data, eu32 flag){

    auto msg = emi_msg_alloc(data_size);
    if(msg == nullptr){
        return emi_msg_ptr{};
    }

    auto p = emi_msg_ptr(msg, emi_msg_free);

    emi_fill_msg(p.get(),dest_ip, data, cmd, msg_num, flag);
    return p;
}

#endif


#endif
