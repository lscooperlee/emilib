
#include <stdlib.h>
#include <stdio.h>

#include "emi_hash.h"


static struct hlist_head test_msg_table_head[EMI_MSG_TABLE_SIZE];

void print_msg_table(){
    int i;
    struct msg_map *map;

    printf("msg_table: size = %lu\n", ARRAY_SIZE(test_msg_table_head));

	for(i=0;i<ARRAY_SIZE(test_msg_table_head);i++){
        struct hlist_head *head = &test_msg_table_head[i];

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

/*
    hash_for_each_safe(test_msg_table_head, i, delnode, map, node){
        printf("\tmap: msg=%d, pid=%d\n", map->msg, map->pid);
    }
*/

    printf("\n");
}

void test_hash(){
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
    
    int i;
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

int main(){
    test_hash();
    return 0;
}
