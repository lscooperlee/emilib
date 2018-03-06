
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

inline static emi_msg_ptr make_emi_msg(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const void *data, eu32 data_size, eu32 flag = 0){

    auto msg = emi_msg_alloc(data_size);
    if(msg == nullptr){
        return emi_msg_ptr{};
    }

    auto p = emi_msg_ptr(msg, emi_msg_free);

    emi_msg_init(p.get(),dest_ip, data, cmd, msg_num, flag);
    return p;
}

template <typename C>
emi_msg_ptr make_emi_msg(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const C& container, eu32 flag = 0){
    return make_emi_msg(dest_ip, msg_num, cmd, container.data(), container.size(), flag);
}

inline static int emi_msg_send(emi_msg_ptr& msg){
    return emi_msg_send(msg.get());
}

template <typename C>
int emi_load_retdata(emi_msg const *msg, const C& container){
    return emi_load_retdata(msg, (void *)container.data(), container.size());
}

inline static emi_retdata *get_next_retdata(emi_msg_ptr& msg, const emi_retdata *data) {
    return get_next_retdata(msg.get(), data);
}

#include <iostream>
struct emi_retdata_iter {
    emi_retdata_iter(const emi_msg *msg_)
        : msg(msg_)
        , data(get_next_retdata(msg, nullptr))
    {
    }

    emi_retdata_iter()
        : emi_retdata_iter(nullptr)
    {
    }

    void operator++(){
        data = get_next_retdata(msg, data);
    }

    const emi_retdata* operator*(){
        return data;
    }

    bool operator!=(const emi_retdata_iter&){
        return data != nullptr;
    }

    emi_retdata_iter(const emi_retdata_iter& other) = delete;
    emi_retdata_iter(emi_retdata_iter&& other) = default;

private:
    const emi_msg *msg;
    emi_retdata *data;
};

struct emi_retdata_container {
    emi_retdata_container(const emi_msg *msg_)
        : msg(msg_)
    {
    }

    emi_retdata_iter begin(){
        return emi_retdata_iter(msg);
    }

    emi_retdata_iter end(){
        return emi_retdata_iter();
    }

private:
    const emi_msg *msg;
};

emi_retdata_container emi_msg::retdata() const {
    return emi_retdata_container(this);
}

const void *emi_msg::data() const {
    return GET_DATA(this);
}

#endif

#endif
