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


int list_add(struct list_node **list, void *pdata);

#endif
