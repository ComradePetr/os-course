#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include "kernel.h"

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

struct vlist_head {
	volatile struct vlist_head *next;
	volatile struct vlist_head *prev;
};

typedef volatile struct vlist_head vlist_head_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
#define LIST_ENTRY(ptr, type, member) CONTAINER_OF(ptr, type, member)

#define VLIST_HEAD(name) struct vlist_head name = LIST_HEAD_INIT(name)


void list_init(struct list_head *head);
void list_add(struct list_head *new, struct list_head *head);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
void list_splice(struct list_head *list, struct list_head *head);
bool list_empty(const struct list_head *head);
struct list_head *list_first(struct list_head *head);
size_t list_size(const struct list_head *head);

void vlist_init(vlist_head_t *head);
void vlist_add(vlist_head_t *new, vlist_head_t *head);
void vlist_add_tail(vlist_head_t *new, vlist_head_t *head);
void vlist_del(vlist_head_t *entry);
void vlist_splice(vlist_head_t *vlist, vlist_head_t *head);
bool vlist_empty(const vlist_head_t *head);
vlist_head_t *vlist_first(vlist_head_t *head);
size_t vlist_size(const vlist_head_t *head);

#endif /*__LIST_H__*/
