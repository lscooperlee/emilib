
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "emi_msg.h"
#include "emi_if.h"

#ifndef SEND_ONLY
#include "emi_ifr.h"
#include "emi_core.h"
#endif



#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
#include <memory>
#include <functional>

using emi_msg_ptr = std::unique_ptr<emi_msg, std::function<decltype(emi_msg_free)>>;

inline static emi_msg_ptr make_emi_msg(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const void *data = nullptr, eu32 data_size = 0, eu32 flag = 0) noexcept {

    auto msg = emi_msg_alloc(data_size);
    if(msg == nullptr){
        return emi_msg_ptr{};
    }

    auto p = emi_msg_ptr(msg, emi_msg_free);

    emi_msg_init(p.get(), dest_ip, msg_num, cmd, flag, data_size, data);

    return p;
}

template <typename C>
emi_msg_ptr make_emi_msg(const char *dest_ip, eu32 msg_num, eu32 cmd, 
        const C& container, eu32 flag = 0) noexcept {
    return make_emi_msg(dest_ip, msg_num, cmd, container.data(), container.size(), flag);
}

inline static int emi_msg_send(const emi_msg_ptr& msg) noexcept {
    return emi_msg_send(msg.get());
}

template <typename C>
int emi_load_retdata(struct emi_msg const *msg, const C& container) noexcept {
    return emi_load_retdata(msg, (void *)container.data(), container.size());
}

inline static struct emi_retdata *get_next_retdata(emi_msg_ptr& msg, struct emi_retdata *data) noexcept {
    return get_next_retdata(msg.get(), data);
}

struct emi_retdata_iter {
    explicit emi_retdata_iter(const struct emi_msg *msg_) noexcept
        : msg(msg_)
        , data(get_next_retdata(msg, nullptr))
    {
    }

    emi_retdata_iter() noexcept
        : msg(nullptr)
        , data(nullptr)
    {
    }

    void operator++() noexcept {
        data = get_next_retdata(msg, data);
    }

    const struct emi_retdata* operator*() const noexcept {
        return data;
    }

    bool operator!=(const emi_retdata_iter&) const noexcept {
        return data != nullptr;
    }

    emi_retdata_iter(const emi_retdata_iter& other) = delete;
    emi_retdata_iter(emi_retdata_iter&& other) = default;

private:
    const emi_msg *msg;
    emi_retdata *data;
};

struct emi_retdata_container {
    explicit constexpr emi_retdata_container(const emi_msg *msg_) noexcept
        : msg(msg_)
    {
    }

    const emi_retdata_iter begin() const noexcept { 
        return emi_retdata_iter(msg);
    }

    const emi_retdata_iter end() const noexcept {
        return emi_retdata_iter();
    }

private:
    const emi_msg *msg;
};

inline emi_retdata_container emi_msg::retdata() noexcept {
    return emi_retdata_container(this);
}
inline void *emi_msg::data() const noexcept {
    return GET_DATA(this);
}

#endif

#endif
