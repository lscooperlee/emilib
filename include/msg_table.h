/*
 EMI:    embedded message interface
 Copyright (C) 2009  Cooper <davidontech@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __MSG_TABLE_H__
#define __MSG_TABLE_H__
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "emi_semaphore.h"
#include "emi_config.h"

#define ARRAY_SIZE(array)    (sizeof(array)/sizeof(array[0]))

static espinlock_t msg_table_lock;

struct msg_map {
    eu32 msg;
    pid_t pid;
    struct msg_map *next;
    struct list_head same;
};

static inline int msg_map_init(struct msg_map *map, eu32 msg, pid_t pid) {
    if (map != NULL) {
        map->msg = msg;
        map->pid = pid;
        map->next = NULL;
        INIT_LIST_HEAD(&map->same);
        return 0;
    }
    return -1;
}

static inline int msg_map_cmp(struct msg_map *map1, struct msg_map *map2) {
    if ((map1 == NULL && map2 != NULL) || (map1 != NULL && map2 == NULL))
        return -1;
    return map1->msg == map2->msg ? 0 : -1;
}

static inline int msg_map_same(struct msg_map *map1, struct msg_map *map2) {
    if (msg_map_cmp(map1, map2))
        return -1;
    return map1->pid == map2->pid ? 0 : -1;
}

static inline eu32 __emi_hash(eu32 k) {
    /* see Understanding the Linux Kernel, P93*/
    eu32 hash = k * 0x9e370001UL;
    return hash >> (32 - EMI_HASH_MASK);
}

static inline eu32 emi_hash(struct msg_map *map) {
    return __emi_hash(map->msg);
}

static inline struct msg_map *__emi_hsearch(struct msg_map *table[],
        struct msg_map *map, int *num) {
    struct msg_map *tmp;
    int i = 0;
    for (i = 0, tmp = *(table + __emi_hash(map->msg));
            (i < *num) && (tmp != NULL);) {
        i++;
        tmp = tmp->next;
    }
    for (; tmp != NULL; tmp = tmp->next) {
        if (!msg_map_cmp(tmp, map)) {
            *num = i;
            return tmp;
        }
    }
    return NULL;
}

static inline int emi_hsearch(struct msg_map *table[],
        struct msg_map *map, struct list_head *head) {

    struct msg_map *same;
    int i = 0, j = 0;

    for (;; i += 1, j++) {
        if ((same = __emi_hsearch(table, map, &i)) != NULL) {
            list_add_tail(&same->same, head);
        } else {
            break;
        }
    }
    return j;
}

static inline int __emi_hinsert(struct msg_map **table, struct msg_map *map) {
    struct msg_map *insert;

    if ((insert = (struct msg_map *) malloc(sizeof(struct msg_map))) == NULL)
        return -1;
    memset(insert, 0, sizeof(struct msg_map));

    insert->msg = map->msg;
    insert->pid = map->pid;
    insert->next = *(table + emi_hash(map));
    *(table + emi_hash(map)) = insert;
    return 0;

}

static inline int emi_hinsert(struct msg_map **table, struct msg_map *map) {
    struct msg_map *tmp;
    for (tmp = *(table + emi_hash(map)); tmp != NULL; tmp = tmp->next) {
        if (!msg_map_same(tmp, map))
            return 1;
    }
    if (__emi_hinsert(table, map) < 0)
        return -1;
    return 0;

}

static inline int emi_hdelete(struct msg_map **table, struct msg_map *map) {
    struct msg_map *deleted, *tmp;
    deleted = *(table + emi_hash(map));
    if (!msg_map_same(deleted, map)) {
        *(table + emi_hash(map)) = deleted->next;
        list_del(&deleted->same);
        free(deleted);
        return 0;
    } else {
        for (; deleted != NULL;) {
            tmp = deleted;
            deleted = deleted->next;
            if (!msg_map_same(deleted, map)) {
                tmp->next = deleted->next;
                list_del(&deleted->same);
                free(deleted);
                return 0;
            }
        }
        return -1;
    }
}

static inline int init_msg_table_lock(struct msg_map *table[]){
    int i;
    for(i=0;i<EMI_MSG_TABLE_SIZE;i++)
        table[i]=NULL;
    return emi_spin_init(&msg_table_lock);
}

static inline int emi_hinsert_lock(struct msg_map **table, struct msg_map *p){
    int ret;
    emi_spin_lock(&msg_table_lock);
    ret = emi_hinsert(table, p);
    emi_spin_unlock(&msg_table_lock);
    return ret;
}

static inline int emi_hdelete_lock(struct msg_map **table, struct msg_map *p){
    int ret;
    emi_spin_lock(&msg_table_lock);
    ret = emi_hdelete(table, p);
    emi_spin_unlock(&msg_table_lock);
    return ret;
}

static inline int emi_hsearch_lock(struct msg_map **table, struct msg_map *p, struct list_head *head){
    int ret;
    emi_spin_lock(&msg_table_lock);
    ret = emi_hsearch(table, p, head);
    emi_spin_unlock(&msg_table_lock);
    return ret;
}

static inline struct msg_map *emi_hsearch_first_lock(struct msg_map **table, struct msg_map *p){
    struct msg_map *mp;
    int tmpnum=0;
    emi_spin_lock(&msg_table_lock);
    mp=__emi_hsearch(table, p, &tmpnum);
    emi_spin_unlock(&msg_table_lock);
    return mp; 
}

#endif

