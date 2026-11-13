/*
 * File: init.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("\nHello world!!\n");
    printf("Hello world!!\n");
    printf("Hello world!!\n");
    write(2, (const void*)"Hello world\n", 12);
    while (true)
    {
    }
    
    return 0;
}