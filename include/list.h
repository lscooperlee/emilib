// see linux kernel list.h
//
#ifndef __LIST_H__
#define __LIST_H__

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new_lst,
                  struct list_head *prev,
                  struct list_head *next)
{
    next->prev = new_lst;
    new_lst->next = next;
    new_lst->prev = prev;
    prev->next = new_lst;
}

static inline void list_add(struct list_head *new_lst, struct list_head *head)
{
    __list_add(new_lst, head, head->next);
}

static inline void list_add_tail(struct list_head *new_lst, struct list_head *head)
{
    __list_add(new_lst, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head * entry)
{
    __list_del(entry->prev,entry->next);
}

static inline void list_del_first_entry(struct list_head * head)
{
    list_del(head->next);
}

static inline void list_replace(struct list_head *old,
                struct list_head *new_lst)
{
    new_lst->next = old->next;
    new_lst->next->prev = new_lst;
    new_lst->prev = old->prev;
    new_lst->prev->next = new_lst;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
                  struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

static inline int list_is_last(const struct list_head *list,
                const struct list_head *head)
{
    return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline int list_is_singular(const struct list_head *head)
{
    return !list_empty(head) && (head->next == head->prev);
}


#ifdef __compiler_offsetof
#define _offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define _offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({            \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - _offsetof(type,member) );})

#define list_entry(ptr, type, member)    container_of(ptr, type, member)

#define list_first_entry(head, type, member) \
    list_entry((head)->next, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_tail(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member)                        \
    for (pos = list_entry((head)->next, __typeof__(*pos), member);        \
         &pos->member != (head);                                     \
         pos = list_entry(pos->member.next, __typeof__(*pos), member))

#define list_for_each_entry_tail(pos, head, member)                        \
    for (pos = list_entry((head)->prev, __typeof__(*pos), member);        \
         &pos->member != (head);                                     \
         pos = list_entry(pos->member.prev, __typeof__(*pos), member))

#endif
