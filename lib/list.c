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

/*
 * TODO list_delete()
 */

#if 0
int main(void)
{
    struct list_node *alist1 = NULL; // must inited to NULL
    struct list_node *alist2 = NULL;
    int a1 = 3;
    int a2 = 5;
    int a3 = 9;
    double b1 = 5.5;
    double b2 = 7.7;
    list_add(&alist1, &a1);
    list_add(&alist1, &a2);
    list_add(&alist1, &a3);

    list_add(&alist2, &b2);
    list_add(&alist2, &b1);

    printf("list1:%d %d %d\n", *(int*)(alist1->pdata), *(int*)(alist1->next->pdata), *(int*)(alist1->next->next->pdata));

    printf("list2:%g %g\n", *(double*)(alist2->pdata), *(double*)(alist2->next->pdata));
    return 0;

}
#endif
