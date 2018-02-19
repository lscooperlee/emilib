#ifndef __EMI_HASH_H__
#define __EMI_HASH_H__
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 


#include "list.h"
#include "emi_lock.h"
#include "emi_config.h"
#include "emi_dbg.h"

#define ARRAY_SIZE(array)    (sizeof(array)/sizeof(array[0]))

inline static eu32 __hash32(eu32 k) {
    /* see Understanding the Linux Kernel, P93*/
    eu32 hash = k * 0x9e370001UL;
    return hash >> (32 - EMI_HASH_MASK);
}

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

inline static void INIT_HLIST_NODE(struct hlist_node *h) {
    h->next = NULL;
    h->pprev = NULL;
}

inline static void hlist_add_head(struct hlist_node *n, struct hlist_head *h) {
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

inline static int hlist_unhashed(const struct hlist_node *h) {
        return !h->pprev;
}

#define hash_add(hashtable, node, key)						\
	hlist_add_head(node, &hashtable[__hash32(key)])

inline static bool hash_hashed(struct hlist_node *node)
{
	return !hlist_unhashed(node);
}

inline static int hlist_empty(const struct hlist_head *h) {
        return !h->first;
}

inline static bool __hash_empty(struct hlist_head *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		if (!hlist_empty(&ht[i]))
			return false;

	return true;
}

inline static void __hlist_del(struct hlist_node *n) {
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

inline static void hlist_del_init(struct hlist_node *n) {
    if (!hlist_unhashed(n)) {
        __hlist_del(n);
        INIT_HLIST_NODE(n);
    }
}

#define hash_empty(hashtable) __hash_empty(hashtable, ARRAY_SIZE(hashtable))

inline static void hash_del(struct hlist_node *node) {
	hlist_del_init(node);
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
	})

#define hlist_for_each_entry(pos, head, member)				\
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_safe(pos, n, head, member) 		\
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
	     pos && ({ n = pos->member.next; 1; });			\
	     pos = hlist_entry_safe(n, typeof(*pos), member))

#define hash_for_each(name, bkt, obj, member)				\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < ARRAY_SIZE(name);\
			(bkt)++)\
		hlist_for_each_entry(obj, &name[bkt], member)

#define hash_for_each_safe(name, bkt, tmp, obj, member)			\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < ARRAY_SIZE(name);\
			(bkt)++)\
		hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)


struct msg_map {
    eu32 msg;
    pid_t pid;
    struct hlist_node node;
};

#define emi_hsearch(name, obj, member, key) \
    hlist_for_each_entry(obj, &name[__hash32(key)], member)

#define emi_hsearch_safe(name, obj, tmp, member, key) \
    hlist_for_each_entry_safe(obj, tmp, &name[__hash32(key)], member)

#define emi_traverse(name, bkt, tmp, obj, member)			\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < ARRAY_SIZE(name);\
			(bkt)++)\
		hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

extern erwlock_t msg_table_lock;
extern struct hlist_head msg_table_head[EMI_MSG_TABLE_SIZE];

int msg_map_init(struct msg_map *map, eu32 msg, pid_t pid);

struct msg_map *alloc_msg_map();
void free_msg_map(struct msg_map *map);

void emi_hinsert(struct hlist_head *table, struct msg_map *map);

void emi_hdelete(struct msg_map *map);

int init_msg_table_lock(struct hlist_head *table, erwlock_t *msg_table_lock);

int emi_hinsert_unique_lock(struct hlist_head *table, struct msg_map *p);

#endif

