#include <emi.h>

#include <memory>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>

#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define _assert(assertion)  do{     \
        if( !(assertion) ){         \
            std::cerr<<__func__<<":"<<__LINE__<<": failed"<<std::endl;\
        } \
    }while(0)

#define ASSERT(x) _assert(x)

const char *ipaddr = "127.0.0.1";

class Process
{
public:
    explicit Process(std::function<void(void)> target_)
        :target(std::move(target_))
        ,pid(-1)
    {
    }

    void Start(){
        pid = fork();
        if(pid == 0){
            target();
            exit(0);
        }
    };

    void Join(){
        waitpid(pid, NULL, 0);
    }

private:
    std::function<void(void)> target;
    pid_t pid;


};

class EmiTestCore
{
    
    FILE *p = nullptr;

public:
    EmiTestCore(){
        p = popen("emi_core -d", "r");
        if(p == nullptr){
            throw "open emi_core failed";
        }
        usleep(1000*500);
    }

    ~EmiTestCore(){
        if(system("killall -9 emi_core")){
            exit(-1);
        }
        pclose(p);
        usleep(1000*500);
    }
    
};

void test_emi_init(){
    ASSERT(emi_init() == 0);
}

void test_emi_msg_register(){
    int ret = -1;
    ASSERT(emi_init() == 0);

    auto func=[](const struct emi_msg *){return 0;};

    ret = emi_msg_register(1, func);
    ASSERT(ret == 0);

    ret = emi_msg_register(1, func);
    ASSERT(ret == -1);

}

void test_emi_msg_send_unblock_nosenddata(){

    auto recvprocess_unblock = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 2);
            ASSERT(msg->cmd == 1);
            return 0;
        };

        int ret = emi_msg_register(2, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess_unblock);
    Process p2(recvprocess_unblock);
    p1.Start();
    p2.Start();

    sleep(1);

    int ret;
    struct emi_msg *msg = emi_msg_alloc(0);
    emi_fill_msg(msg, ipaddr, NULL, 1, 2, 0);
    ret = emi_msg_send(msg);

    ASSERT(ret == 0);

    p1.Join();
    p2.Join();

}

void test_emi_msg_send_unblock_senddata(){

    auto recvprocess_unblock_data = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 3);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "11112222", msg->size) == 0);
            return 0;
        };

        int ret = emi_msg_register(3, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess_unblock_data);
    Process p2(recvprocess_unblock_data);
    p1.Start();
    p2.Start();

    sleep(1);

    int ret;
    struct emi_msg *msg = emi_msg_alloc(strlen("11112222"));
    emi_fill_msg(msg, ipaddr, "11112222", 1, 3, 0);
    ret = emi_msg_send(msg);

    ASSERT(ret == 0);

    p1.Join();
    p2.Join();

    emi_msg_free(msg);

}

void test_emi_msg_send_block_noretdata(){

    auto recvprocess_block= [](){
        emi_init();
        usleep(10000);

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 4);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "11112222", msg->size) == 0);
            return 0;
        };

        int ret = emi_msg_register(4, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess_block);
    Process p2(recvprocess_block);
    p1.Start();
    p2.Start();

    sleep(1);

    int ret;
    struct emi_msg *msg = emi_msg_alloc(strlen("11112222"));
    emi_fill_msg(msg, ipaddr, "11112222", 1, 4, EMI_MSG_MODE_BLOCK);
    ret = emi_msg_send(msg);

    ASSERT(ret == 0);

    p1.Join();
    p2.Join();

    sleep(1);

    auto recvprocess_block_fail = [](){
        emi_init();
        usleep(10000);

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 4);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "11112222", msg->size) == 0);
            return -1;
        };

        int ret = emi_msg_register(4, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p3(recvprocess_block);
    Process p4(recvprocess_block);
    Process p5(recvprocess_block_fail);
    p3.Start();
    p4.Start();
    p5.Start();

    sleep(1);

    emi_fill_msg(msg, ipaddr, "11112222", 1, 4, EMI_MSG_MODE_BLOCK);
    ret = emi_msg_send(msg);

    ASSERT(ret == -1);

    p3.Join();
    p4.Join();
    p5.Join();

    emi_msg_free(msg);
}

void test_emi_msg_send_block_retdata(){

    auto recvprocess_block_data = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 5);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "11112222", msg->size) == 0);
            char *retdata = (char *)emi_retdata_alloc(msg, 8);
            if(retdata != NULL)
                strncpy(retdata, "aaaabbbb", 8);

            return 0;
        };

        int ret = emi_msg_register(5, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess_block_data);
    Process p2(recvprocess_block_data);
    p1.Start();
    p2.Start();

    sleep(1);

    int ret;
    struct emi_msg *msg = emi_msg_alloc(strlen("11112222"));
    emi_fill_msg(msg, ipaddr, "11112222", 1, 5, EMI_MSG_MODE_BLOCK);
    ret = emi_msg_send(msg);

    ASSERT(ret == 0);
    ASSERT(strncmp((char *)GET_RETDATA(msg), "aaaabbbb", msg->retsize) == 0);

    p1.Join();
    p2.Join();

    emi_msg_free(msg);
}

template <int N>
std::array<char, N> get_compbuf(){ 
    std::array<char, N> compbuf;
    std::generate(compbuf.begin(), compbuf.end(), [](){return 'c';});
    return compbuf;
}

void test_emi_msg_send_block_retdata_large_data(){

    auto recvprocess_block_data = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 5);
            ASSERT(msg->cmd == 1);
            auto d = get_compbuf<1024>().data();
            ASSERT(strncmp((char *)GET_DATA(msg), d, msg->size) == 0);
            char *retdata = (char *)emi_retdata_alloc(msg, 8);
            if(retdata != NULL)
                strncpy(retdata, "aaaabbbb", 8);

            return 0;
        };

        int ret = emi_msg_register(5, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess_block_data);
    Process p2(recvprocess_block_data);
    p1.Start();
    p2.Start();

    sleep(1);

    int ret;
    struct emi_msg *msg = emi_msg_alloc(1024);
    auto d = get_compbuf<1024>().data();
    emi_fill_msg(msg, ipaddr, d, 1, 5, EMI_MSG_MODE_BLOCK);
    ret = emi_msg_send(msg);

    ASSERT(ret == 0);
    ASSERT(strncmp((char *)GET_RETDATA(msg), "aaaabbbb", msg->retsize) == 0);

    p1.Join();
    p2.Join();

    emi_msg_free(msg);
}

void test_emi_cpp(){
    using namespace std::string_literals;
    emi_init();

    auto func=[](const struct emi_msg *msg){
        ASSERT(msg->msg == 6);
        ASSERT(msg->cmd == 1);
        ASSERT(strncmp((char *)GET_DATA(msg), "helloworld", msg->size) == 0);
        return emi_load_retdata(msg, "12345678", 8);
    };

    int ret = emi_msg_register(6, func);
    ASSERT(ret == 0);

    auto msg_ptr = make_emi_msg_ptr("127.0.0.1", 6, 1, "helloworld"s, EMI_MSG_MODE_BLOCK);

    ret = emi_msg_send(msg_ptr);
    ASSERT(ret == 0);
    ASSERT(strncmp((char *)GET_RETDATA(msg_ptr), "12345678", msg_ptr->retsize) == 0);
}

void test_emi_some_receiver_exit(){

    auto recvprocess1 = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 7);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "helloworld", msg->size) == 0);
            return 0;
        };

        int ret = emi_msg_register(7, func);
        ASSERT(ret == 0);
    };

    auto recvprocess2 = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 7);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "helloworld", msg->size) == 0);
            return 0;
        };

        int ret = emi_msg_register(7, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess1);
    p1.Start();  //keep a invalid record of msg_map in emi_core
    p1.Join();

    Process p2(recvprocess2);
    p2.Start();

    sleep(1);

    auto msg_ptr = make_emi_msg_ptr("127.0.0.1", 7, 1, "helloworld", 
            sizeof("helloworld"), EMI_MSG_MODE_BLOCK);

    if(msg_ptr){
        int ret = emi_msg_send(msg_ptr);
        ASSERT(ret == 0);
    }

    p2.Join();
}

void test_emi_all_receiver_exit(){

    auto recvprocess = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 8);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "helloworld", msg->size) == 0);
            return 0;
        };

        int ret = emi_msg_register(8, func);
        ASSERT(ret == 0);
    };

    Process p1(recvprocess);
    p1.Start();  //keep a invalid record of msg_map in emi_core
    p1.Join();

    Process p2(recvprocess);
    p2.Start();
    p2.Join();

    sleep(1);

    auto msg_ptr = make_emi_msg_ptr("127.0.0.1", 7, 1, "helloworld", 
            sizeof("helloworld"), EMI_MSG_MODE_BLOCK);

    if(msg_ptr){
        int ret = emi_msg_send(msg_ptr);
        ASSERT(ret == -1);
    }
}

void test_emi_exit(){
    auto recvprocess = [](){
        emi_init();

        auto func=[](const struct emi_msg *msg){
            ASSERT(msg->msg == 6);
            ASSERT(msg->cmd == 1);
            ASSERT(strncmp((char *)GET_DATA(msg), "helloworld", msg->size) == 0);
            exit(0);
            return 0;
        };

        int ret = emi_msg_register(6, func);
        ASSERT(ret == 0);

        pause();
        sleep(1);
    };

    Process p1(recvprocess);
    p1.Start();
    sleep(1);

    auto msg_ptr = make_emi_msg_ptr("127.0.0.1", 6, 1, "helloworld", sizeof("helloworld"), 0);
    if(msg_ptr){
        int ret = emi_msg_send(msg_ptr);
        ASSERT(ret == 0);
    }

    p1.Join();
}

using testFuncType = void(*)();

std::vector<std::pair<std::string, testFuncType>> testsVector = {
    {"emi_init", test_emi_init},
    {"emi_msg_register", test_emi_msg_register},
    {"emi_msg_send_unblock_nosenddata", test_emi_msg_send_unblock_nosenddata},
    {"emi_msg_send_unblock_senddata", test_emi_msg_send_unblock_senddata},
    {"emi_msg_send_block_noretdata", test_emi_msg_send_block_noretdata},
    {"emi_msg_send_block_retdata", test_emi_msg_send_block_retdata},
    {"emi_msg_send_block_retdata_large_data", test_emi_msg_send_block_retdata_large_data},
    {"emi_cpp", test_emi_cpp},
    {"emi_some_receiver_exit", test_emi_some_receiver_exit},
    {"emi_all_receiver_exit", test_emi_all_receiver_exit},
    {"emi_exit", test_emi_exit},
};

void Run(void){

    for(auto f: testsVector){
        EmiTestCore core;
        auto prompt = std::get<0>(f);
        std::cout<<"testing "<<prompt<<std::endl;
        std::get<1>(f)();
    }
}


int main(){
    Run();
    return 0;
}
