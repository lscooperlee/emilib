
#include "emi_hash.h"

erwlock_t msg_table_lock;
struct hlist_head msg_table_head[EMI_MSG_TABLE_SIZE];

struct msg_map *alloc_msg_map() {
    return (struct msg_map *)malloc(sizeof(struct msg_map));
}
void free_msg_map(struct msg_map *map) {
    //FIXME should be atomic
    if(map != NULL){
        emi_hdelete(map);
        free(map);
    }
}

int msg_map_init(struct msg_map *map, eu32 msg, pid_t pid) {
    if (map != NULL) {
        map->msg = msg;
        map->pid = pid;
        INIT_HLIST_NODE(&map->node);
        return 0;
    }

    return -1;
}

void emi_hinsert(struct hlist_head *table, struct msg_map *map) {
    hash_add(table, &map->node, map->msg);
}

void emi_hdelete(struct msg_map *map) {
    hash_del(&map->node);
}

int init_msg_table_lock(struct hlist_head *table, erwlock_t *msg_table_lock){
    int i;
    for(i=0;i<EMI_MSG_TABLE_SIZE;i++){
        INIT_HLIST_HEAD(&table[i]);
    }
    return emi_rwlock_init(msg_table_lock);
}

int emi_hinsert_unique_lock(struct hlist_head *table, struct msg_map *p){
    struct msg_map *map=NULL;

    hlist_for_each_entry(map, &table[__hash32(p->msg)], node) {
        if(map->pid == p->pid){
            return -1;
        }
    }
    emi_rwlock_wrlock(&msg_table_lock);
    emi_hinsert(table, p);
    emi_rwlock_unlock(&msg_table_lock);

    return 0;
}

