
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "emi_if.c"
#include "emi_ifr.c"
#include "../catch.hpp"


TEST_CASE("emi_msg_send"){

    auto receiver = [](auto&& container, std::promise<void>& promise){
            int ret;

            int sd = socket(AF_INET, SOCK_STREAM, 0);
            REQUIRE(sd > 0);
            
            int on = 1;
            ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
            REQUIRE(ret == 0);

            sockaddr_in m_addr;

            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = INADDR_ANY;
            m_addr.sin_port = htons(1361);

            ret = bind(sd, (struct sockaddr *)&m_addr, sizeof(m_addr));
            REQUIRE(ret == 0);

            ret = listen(sd, 1);
            REQUIRE(ret == 0);

            promise.set_value();

            int client = accept(sd, nullptr, nullptr);
            REQUIRE(client > 0);

            size_t ret_size;
            struct emi_msg msg;
            ret_size = read(client, &msg, EMI_MSG_PAYLOAD_SIZE);
            REQUIRE(ret_size == EMI_MSG_PAYLOAD_SIZE);
            
            CHECK(msg.msg == 2);
            CHECK(msg.cmd == 1);
            
            char received_data[msg.size];
            ret_size = read(client, received_data, msg.size);
            REQUIRE(ret_size == msg.size);

            eu32 size = sizeof(decltype(container[0])) * container.size();
            auto data_addr = container.data();
            CHECK(msg.size == size);
            CHECK(memcmp(data_addr, received_data, msg.size) == 0);
            
            if(msg.flag & EMI_MSG_MODE_BLOCK){
                eu32 retsize = sizeof(struct emi_retdata) + msg.size;
                msg.flag |= EMI_MSG_RET_SUCCEEDED;
                msg.retsize = 2 * retsize;
                ret_size = write(client, &msg, EMI_MSG_PAYLOAD_SIZE);
                REQUIRE(ret_size == EMI_MSG_PAYLOAD_SIZE);

                for(int i=0; i<2; ++i){
                    char retdata_buf[retsize];
                    struct emi_retdata *retdata = reinterpret_cast<struct emi_retdata *>(retdata_buf);
                    retdata->size = msg.size;
                    memcpy(retdata->data, received_data, msg.size);

                    ret_size = write(client, retdata, retsize);
                    REQUIRE(ret_size == retsize);
                }
            }

            close(client);
            close(sd);
    };

    auto sender = [](auto&& container, std::future<void> future, eu32 flag=0){
        int size = sizeof(decltype(container[0])) * container.size();
        auto data_addr = container.data();

        struct emi_msg *msg = emi_msg_alloc(size);
        emi_msg_init(msg, "127.0.0.1", data_addr, 1, 2, flag);

        future.wait();

        int ret = emi_msg_send(msg);
        CHECK(ret == 0);

        if(flag & EMI_MSG_MODE_BLOCK){
            struct emi_retdata *retdata;
            for_each_retdata(msg, retdata){
                CHECK(memcmp(retdata, data_addr, retdata->size));
            }
        }

    };

    SECTION("emi_msg_send without data"){
        const std::vector<char> data;
        std::promise<void> promise;
        std::thread rthread(receiver, data, std::ref(promise));
        std::thread sthread(sender, data, promise.get_future());

        rthread.join();
        sthread.join();
    }

    SECTION("emi_msg_send with send data"){
        const std::vector<char> data={'t','m','q','n'};
        std::promise<void> promise;
        std::thread rthread(receiver, data, std::ref(promise));
        std::thread sthread(sender, data, promise.get_future());

        rthread.join();
        sthread.join();
    }

    SECTION("emi_msg_send with ret data"){
        const std::vector<char> data={'t','m','q','n'};
        std::promise<void> promise;
        std::thread rthread(receiver, data, std::ref(promise));
        std::thread sthread(sender, data, promise.get_future(), EMI_MSG_MODE_BLOCK);

        rthread.join();
        sthread.join();
    }
}
