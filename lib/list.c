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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
 * Linus Torvalds : how to delete a node using a pointer to pointer
 * http://meta.slashdot.org/story/12/10/11/0030249/linus-torvalds-answers-your-questions
 */
void list_destroy(struct list_node **list)
{
    assert(list != NULL);
    struct list_node *entry;
    struct list_node **curr = list;
    while(*curr){
        entry = *curr;
        free( entry->pdata );
        *curr = entry->next;
        free(entry);
    }
    *list = NULL;
}


void list_delete_byfeature(struct list_node **list, int(*pfun)(struct list_node *, void *), void *data)
{
    assert(list && pfun);
    struct list_node **curr = list;
    struct list_node *entry;
    while(*curr){
        entry =  *curr;
        if(pfun(entry, data)){
            free(entry->pdata);
            *curr = entry->next;
            free(entry);
        }
        else{
            curr = &entry->next;
        }
    }
}

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

int is_delete(struct list_node *s, void *data)
{
    return *(int*)(s->pdata) == (int)data;
}


int traverse_print(struct list_node *list)
{
    printf("%d, ", *(int*)list->pdata);
    return 0;
}

int main(void)
{
    struct list_node *alist1 = NULL; // must inited to NULL
    struct list_node *alist2 = NULL;
    int *a1 = (int*)malloc(sizeof(int));
    *a1 = 3;
    int *a2 = (int*)malloc(sizeof(int));
    *a2 = 5;
    int *a3 = (int*)malloc(sizeof(int));
    *a3 = 9;

    // search empty list
    struct list_node *pa = list_search(alist1, search_int, (void*)*a1);
    assert(pa == NULL);

    // search 1-item list
    list_add(&alist1, (void*)a1);
    pa = list_search(alist1, search_int, (void*)*a1);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    // search 2-item list
    list_add(&alist1, (void*)a2);
    pa = list_search(alist1, search_int, (void*)*a2);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    // test list_traverse
    list_traverse(alist1, traverse_add1);
    pa = list_search(alist1, search_int, (void*)3);
    assert(pa == NULL);

    pa = list_search(alist1, search_int, (void*)*a1);
    assert(pa);
    printf("found:%d\n", *(int*)pa->pdata);

    list_add(&alist1, (void*)a3);


    printf("full list:");list_traverse(alist1, traverse_print); printf("\n");

    list_delete_byfeature(&alist1, is_delete, (void*)*a1);
    printf("after delete 4:");list_traverse(alist1, traverse_print); printf("\n");

    list_delete_byfeature(&alist1, is_delete, (void*)*a3);
    printf("after delete 9:");list_traverse(alist1, traverse_print); printf("\n");

    list_delete_byfeature(&alist1, is_delete, (void*)*a2);
    printf("after delete 6:");list_traverse(alist1, traverse_print); printf("\n");

    // delte a non-exsit note
    list_delete_byfeature(&alist1, is_delete, (void*)6);
    printf("after delte non-exist node:");list_traverse(alist1, traverse_print); printf("\n");

    // test different type
    double *b1 = (double*)malloc(sizeof(double));
    *b1 = 5.5;
    double *b2 = (double*)malloc(sizeof(double));
    *b2 = 7.7;

    list_add(&alist2, (void*)b2);
    list_add(&alist2, (void*)b1);
    printf("list2:%g %g\n", *(double*)(alist2->pdata), *(double*)(alist2->next->pdata));

    list_destroy(&alist1);
    assert(alist1 == NULL);

    return 0;
}
#endif
