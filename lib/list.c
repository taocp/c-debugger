/*
 * =====================================================================================
 *
 *       Filename:  list.c
 *
 *    Description:  a link list implemention for xibugger
 *
 *        Version:  1.0
 *        Created:  07/28/2014 10:52:40 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datastruct.h"


/*
 * return 0 : successful
 * return -1: error with bad args
 *
 * ================================================================
 * NOTE: the link list MUST **initialized to NULL** : *list == NULL
 * ================================================================
 */
int list_add(struct list_node **list, void *pdata)
{
    if(list == NULL || pdata == NULL){
        return -1;
    }
    struct list_node *pnode = (struct list_node*)malloc(sizeof(struct list_node));
    assert(pnode);
    pnode->pdata = pdata;
    pnode->next = *list;
    *list = pnode;
    return 0;
}

struct list_node *list_search(struct list_node *list, int (*pfun)(struct list_node *, void *), void *data)
{
    assert(pfun && data);
    struct list_node * s = list;
    while(s){
        if( pfun(s, data) ){
            return s;
        }
        s = s->next;
    }
    return s;
}

void list_traverse(struct list_node *list, int (*handle)(struct list_node *))
{
    assert(handle);
    struct list_node * s = list;
    while(s){
        handle(s);
        s = s->next;
    }
}

/*
 * TODO list_delete()
 */

#if 0
int search_int(struct list_node *n, void *d)
{
    return *(int*)(n->pdata) == (int)d;
}

int traverse_add1(struct list_node *list)
{
    (*(int*)list->pdata)++;
    return 0;
}

int main(void)
{
    struct list_node *alist1 = NULL; // must inited to NULL
    struct list_node *alist2 = NULL;
    int a1 = 3;
    int a2 = 5;
    int a3 = 9;

    // search empty list
    struct list_node *pa = list_search(alist1, search_int, (void*)a1);
    assert(pa == NULL);

    // search 1-item list
    list_add(&alist1, &a1);
    pa = list_search(alist1, search_int, (void*)a1);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    // search 2-item list
    list_add(&alist1, &a2);
    pa = list_search(alist1, search_int, (void*)a2);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    // test list_traverse
    list_traverse(alist1, traverse_add1);
    pa = list_search(alist1, search_int, (void*)3);
    assert(pa == NULL);

    pa = list_search(alist1, search_int, (void*)a1);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    list_add(&alist1, &a3);

    // test different type
    double b1 = 5.5;
    double b2 = 7.7;
    list_add(&alist2, &b2);
    list_add(&alist2, &b1);
    printf("list2:%g %g\n", *(double*)(alist2->pdata), *(double*)(alist2->next->pdata));

    return 0;
}
#endif
