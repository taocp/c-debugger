/*
 * =====================================================================================
 *
 *       Filename:
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/16/2014 04:54:41 PM
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

//0804843c
void catch_me(void)
{
    printf("Oop... \n");
}
 
int main(void)
{
    int i;
    for (i = 0; i < 3; ++i){
        catch_me();
    }
    printf(">_<~!\n");
    return 0;
}
