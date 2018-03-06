
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "emi_config.c"
#include "emi_dbg.c"
#include "emi_shmem.c"
#include "emi_sock.c"
#include "emi_sockr.c"
#include "../catch.hpp"


TEST_CASE("emi_read"){

    auto receiver = [](auto&& container, std::promise<unsigned short>& promise){
            int ret;

            int sd = socket(AF_INET, SOCK_STREAM, 0);
            REQUIRE(sd > 0);
            
            int on = 1;
            ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
            REQUIRE(ret == 0);

            sockaddr_in m_addr;

            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = INADDR_ANY;
            m_addr.sin_port = 0;

            ret = bind(sd, (struct sockaddr *)&m_addr, sizeof(m_addr));
            REQUIRE(ret == 0);

            unsigned int socklen = sizeof(m_addr);
            getsockname(sd, (struct sockaddr *)&m_addr, &socklen);

            ret = listen(sd, 1);
            REQUIRE(ret == 0);

            promise.set_value(m_addr.sin_port);

            int client = accept(sd, nullptr, nullptr);
            REQUIRE(client > 0);

            int size = sizeof(decltype(container[0])) * container.size();
            auto data_addr = container.data();

            void *received_addr = malloc(size);
            REQUIRE(received_addr != nullptr);

            struct sk_dpr emi_sd = {client, AF_INET};
            int ret_size = emi_read(&emi_sd, received_addr, size);
            REQUIRE(ret_size == 0);

            CHECK(memcmp(data_addr, received_addr, size) == 0);
            
            free(received_addr);
            close(client);
            close(sd);
    };

    auto sender = [](auto&& container, std::future<unsigned short> future){

        future.wait();
        auto port = future.get();

        int sd = socket(AF_INET, SOCK_STREAM, 0);
        REQUIRE(sd > 0);
        
        sockaddr_in m_addr;
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        m_addr.sin_port = port;

        int ret = connect(sd, (struct sockaddr *)&m_addr, sizeof(m_addr));
        REQUIRE(ret == 0);
        
        int size = sizeof(decltype(container[0])) * container.size();
        auto data_addr = container.data();

        auto write_size = write(sd, data_addr, size);
        REQUIRE(write_size == size);

        close(sd);
    };

    SECTION("emi_read"){

        std::vector<int> data(8192);
        std::generate(data.begin(), data.end(), std::rand);

        std::promise<unsigned short> promise;
        std::thread rthread(receiver, std::ref(data), std::ref(promise));
        std::thread sthread(sender, std::ref(data), promise.get_future());

        rthread.join();
        sthread.join();
    }

}
