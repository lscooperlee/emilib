
#include <stdlib.h>
#include <stdio.h>
#include <set>

#define EMI_HASH_MASK 3

#include "emi_hash.c"
#include "../catch.hpp"


bool check_msg_table(struct hlist_head msg_table[],
        const std::set<std::tuple<unsigned int, eu32, pid_t>>& map = {}){

    unsigned int i;
    struct msg_map *m;

    std::set<std::tuple<unsigned int, eu32, pid_t>> hashed;

	for(i=0; i<EMI_MSG_TABLE_SIZE; i++){
        struct hlist_head *head = &msg_table[i];

        if(!hlist_empty(head)){
            hlist_for_each_entry(m, head, node){
                hashed.emplace(std::make_tuple(i, m->msg, m->pid));
            }
        }
	}

    return hashed == map;
}

void print_msg_table(struct hlist_head msg_table[]){
    int i;
    struct msg_map *map;

    printf("msg_table: size = %u\n", EMI_MSG_TABLE_SIZE);

	for(i=0; i<EMI_MSG_TABLE_SIZE; i++){
        struct hlist_head *head = &msg_table[i];

		printf("\t%d: ", i);
        if(hlist_empty(head)){
		    printf("empty\n");
        }else{
            hlist_for_each_entry(map, head, node){
                printf("map: msg=%u, pid=%u\t", map->msg, map->pid);
            }
            printf("\n");
        }
	}

    printf("\n");
}

void test_hash(){
    struct hlist_head test_msg_table_head[EMI_MSG_TABLE_SIZE];

    int ret = init_msg_table_lock(test_msg_table_head, &msg_table_lock);
    printf("init \n");

    struct msg_map map1;
    msg_map_init(&map1, 13, 29);
    emi_hinsert(test_msg_table_head, &map1);

    struct msg_map map2;
    msg_map_init(&map2, 1, 10);
    emi_hinsert(test_msg_table_head, &map2);

    struct msg_map map3;
    msg_map_init(&map3, 19, 1);
    emi_hinsert_unique_lock(test_msg_table_head, &map3);

    struct msg_map map4;
    msg_map_init(&map4, 13, 1);
    emi_hinsert_unique_lock(test_msg_table_head, &map4);

    struct msg_map map5;
    msg_map_init(&map5, 13, 99);
    emi_hinsert(test_msg_table_head, &map5);

    print_msg_table(test_msg_table_head);

    struct msg_map *map=NULL;
	struct hlist_node *tmp;

    emi_hsearch_safe(test_msg_table_head, map, tmp, node, 13){
        if(map->msg == 13){
            emi_hdelete(map);
        }
    }
    print_msg_table(test_msg_table_head);

    emi_hinsert(test_msg_table_head, &map4);
    print_msg_table(test_msg_table_head);
    
    unsigned int i;
    emi_traverse(test_msg_table_head, i, tmp, map, node){
        if(map->pid == 1){
            emi_hdelete(map);
        }
    }
    print_msg_table(test_msg_table_head);

    struct msg_map map6;
    msg_map_init(&map6, 1, 10);
    ret = emi_hinsert_unique_lock(test_msg_table_head, &map6);
    printf("%d\n", ret);
}


TEST_CASE("emi_hinsert") {

    struct hlist_head test_msg_table_head[EMI_MSG_TABLE_SIZE];
    erwlock_t test_msg_table_lock;

    int ret = init_msg_table_lock(test_msg_table_head, &test_msg_table_lock);
    CHECK(ret == 0);

    CHECK(check_msg_table(test_msg_table_head));

    SECTION("emi_hinsert 1") {
        struct msg_map map = {1, 11, {NULL, NULL}};
        emi_hinsert(test_msg_table_head, &map);
        CHECK(check_msg_table(test_msg_table_head, {{4, 1, 11}}));
    }

    SECTION("emi_hinsert different key") {
        struct msg_map map = {3, 15, {NULL, NULL}};
        struct msg_map map1 = {1, 11, {NULL, NULL}};
        emi_hinsert(test_msg_table_head, &map);
        emi_hinsert(test_msg_table_head, &map1);
        CHECK(check_msg_table(test_msg_table_head, {{4, 1, 11}, {6, 3, 15}}));
    }

    SECTION("emi_hinsert same key") {
        struct msg_map map = {3, 15, {NULL, NULL}};
        struct msg_map map1 = {1, 11, {NULL, NULL}};
        struct msg_map map2 = {1, 15, {NULL, NULL}};
        emi_hinsert(test_msg_table_head, &map);
        emi_hinsert(test_msg_table_head, &map1);
        emi_hinsert(test_msg_table_head, &map2);
        CHECK(check_msg_table(test_msg_table_head, 
                    {{4, 1, 11}, {4, 1, 15}, {6, 3, 15}}));
    }
}

TEST_CASE("emi_hdelete") {

    struct hlist_head test_msg_table_head[EMI_MSG_TABLE_SIZE];
    erwlock_t test_msg_table_lock;

    int ret = init_msg_table_lock(test_msg_table_head, &test_msg_table_lock);
    CHECK(ret == 0);
    CHECK(check_msg_table(test_msg_table_head));

    SECTION("emi_delete 1") {
        struct msg_map map = {1, 11, {NULL, NULL}};
        emi_hinsert(test_msg_table_head, &map);
        CHECK(check_msg_table(test_msg_table_head, {{4, 1, 11}}));

        emi_hdelete(&map);
        CHECK(check_msg_table(test_msg_table_head));
    }

    SECTION("emi_hinsert different key") {
        struct msg_map map = {3, 15, {NULL, NULL}};
        struct msg_map map1 = {1, 11, {NULL, NULL}};
        emi_hinsert(test_msg_table_head, &map);
        emi_hinsert(test_msg_table_head, &map1);
        CHECK(check_msg_table(test_msg_table_head, {{4, 1, 11}, {6, 3, 15}}));

        emi_hdelete(&map);
        CHECK(check_msg_table(test_msg_table_head, {{4, 1, 11}}));
        emi_hdelete(&map1);
        CHECK(check_msg_table(test_msg_table_head));
    }
}
