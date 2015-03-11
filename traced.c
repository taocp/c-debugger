/*
 * =====================================================================================
 *
 *       Filename:  traced.c
 *
 *    Description:  a test for debugger.
 *                  this isn't a part of debugger.
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void catch(void)
{
    printf("catch\n");
}

void capture(void)
{
    printf("capture\n");
}

void grip(void)
{
    printf("grip\n");
}

int main(void)
{
    grip();
    catch();
    capture();
    catch();
    printf(">_<~!\n");
    return 0;
}
