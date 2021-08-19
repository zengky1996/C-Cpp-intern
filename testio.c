/*
 * Usage: ./XXX arg1 arg2 arg3
 * arg1: mode: R(read) or W(write)
 * arg2: size of memory forIO test 
 * arg3: unit of momory: MB/GB
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc,char *argv[])
{
    /* Receive storage parameters */
    char *mode = argv[1];
    int size = atoi(argv[2]);
    char *unit = argv[3];
    int usize = 0;

    printf("unit : %s\n",unit);
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


    /* Test whether the file exists, if it does not exist, create it */
    int fd_w = open("iotest.txt",O_DIRECT|O_CREAT|O_TRUNC|O_WRONLY,0666);
    if(fd_w < 0)
    {
        perror("Fail to open iotest.txt/write");
        return -1;
    }
    
    /* Write data of the specified size to the file */

    int write_size = usize*size;
    char *write_buff = valloc(write_size*sizeof(char));
    memset(write_buff,'$',write_size);
    
    int ret_w = 0;
    while(write_size > 0)
    {
        ret_w = write(fd_w,write_buff,write_size);
        printf("written %d bytes\n",ret_w);
        if(ret_w < 0)
        {
            printf("write error: %s\n",strerror(errno));
            return 0;
        }
    write_size -= ret_w;
    if(write_size == 0) break;
    }
    close(fd_w);

    /* Read the specified size data from the file */ 
    int fd_r = open("iotest.txt",O_DIRECT|O_EXCL|O_RDONLY,0666);
    if(fd_r < 0)
    {
        perror("Fail to open iotest.txt/read");
        return -1;
    }
    int read_size = usize*size;
    char *read_buff = valloc(read_size*sizeof(char));
    int ret_r = 0;
    int read_sum = 0;
    if (!(strncasecmp(mode,"R",1)))
    {
        while((ret_r = read(fd_r,read_buff,40960)))
        {
            //printf("read %d bytes\n",ret_r);
            read_sum += ret_r;
            if(ret_r < 0)
            {
                printf("read error: %s\n",strerror(errno));
                return 0;
            }
        }
        printf("read IO :%d byte\n",read_sum);
        close(fd_r);
    }
    return 0;
}