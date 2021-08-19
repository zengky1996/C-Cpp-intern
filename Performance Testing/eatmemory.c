/*
 * Usage: ./XXX arg1 arg2
 * arg1: size of memory
 * arg2: unit of memory malloc 
 *       B/KB/MB/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define page (4*1024)

int main(int argc,char *argv[])
{
    int size = atof(argv[1]);
    char *unit = argv[2];
    int usize = 0;

    if (!(strncasecmp(unit,"MB",2)))
    {
        usize = 1024*1024;
        printf("input %d MB\n",size);
    }else if (!(strncasecmp(unit,"GB",2)))
    {
        usize = 1024*1024*1024;
        printf("input %d GB\n",size);
    }else{
        perror("Input unit incorrect!\n");
        exit(0);
    }

    char *mem = (char *)malloc(sizeof(char)*usize*size);
    int memsetPages = (size*usize)/page;
    int memsetReset = (size*usize)%page;
    int totalPages = 0;

    while(1)
    {
        char n = 0;
        if(totalPages != memsetPages) {
            memset(mem + totalPages*page,n,page);
        } else {
            if(memsetReset != 0) {
                memset(mem + totalPages*page,n,memsetReset);
            }
            totalPages = 0;
            continue;
        }
        totalPages++;
        n++;
        usleep(1000);
    }

    return 0;
}