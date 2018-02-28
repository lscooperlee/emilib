
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

inline static emi_msg_ptr make_emi_msg_ptr(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const void *data, eu32 data_size, eu32 flag = 0){

    auto msg = emi_msg_alloc(data_size);
    if(msg == nullptr){
        return emi_msg_ptr{};
    }

    auto p = emi_msg_ptr(msg, emi_msg_free);

    emi_fill_msg(p.get(),dest_ip, data, cmd, msg_num, flag);
    return p;
}

template <typename C>
emi_msg_ptr make_emi_msg_ptr(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const C& container, eu32 flag = 0){
    return make_emi_msg_ptr(dest_ip, msg_num, cmd, container.data(), container.size(), flag);
}

inline static int emi_msg_send(emi_msg_ptr& msg){
    return emi_msg_send(msg.get());
}

inline static void *GET_RETDATA(emi_msg_ptr& msg){
    return GET_RETDATA(msg.get());
}

template <typename C>
int emi_load_retdata(struct emi_msg const *msg, const C& container){
    return emi_load_retdata(msg, (void *)container.data(), container.size());
}

#endif


#endif
