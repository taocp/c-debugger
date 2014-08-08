/*
 * =====================================================================================
 *
 *       Filename:  datastruct.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/28/2014 11:22:35 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef DATASTRUCT_H_INCLUDE
#define DATASTRUCT_H_INCLUDE

struct list_node {
    void *pdata;
    struct list_node *next;
};

/*
 * return 0 : successful
 * return -1: error with bad args
 *
 * ================================================================
 * NOTE: the link list MUST **initialized to NULL** : *list == NULL
 * ================================================================
 */
int list_add(struct list_node **list, void *pdata);

struct list_node *list_search(struct list_node *list, int (*pfun)(struct list_node *, void *), void *data);

void list_traverse(struct list_node *list, int (*handle_func)(struct list_node *));

void list_delete_byfeature(struct list_node **list, int(*pfun)(struct list_node *, void *), void *data);

void list_destroy(struct list_node **list);

#endif
