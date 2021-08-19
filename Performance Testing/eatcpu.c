/*
 * Usage: ./XXX arg1 arg2 arg3 arg4 ...
 * arg1: No.X CPU
 * arg2: No.X CPU
 * arg3: No.X CPU    
 * arg4: No.X CPU        
*/

#define _GNU_SOURCE             
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
 
/* Thread function, receiving the parameter N of pthread_create() */
void* testfunc(void* arg) 
{
  int cpuNo = (int)arg;
  cpu_set_t mask;
  CPU_ZERO(&mask);//Clear the mask before assignment
  CPU_SET(cpuNo, &mask);//The mask is assigned to N
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &mask) ;//Bind the thread to cpu N
  printf("pthread_setaffinity_np cpu%d,thread%d\n",cpuNo,cpuNo);
  int i = 1;
  while(i) {
     printf("tid=%d,cpu=%d\n",pthread_self(), sched_getcpu());
     i--;
  }
  while(1);
  return NULL; 
}
 
int main(int argc,char *argv[])
{
  int threadNum = argc - 1;
  pthread_t thread[threadNum];
  int cpu[threadNum];
  int i;
  for (i = 1; i <= threadNum;++i)
  {
    cpu[i] = atoi(argv[i]);
  }
  
  /* Thread creation loop */
  int n;
  for( n = 1;n <= threadNum;++n)
  {
    int ret;
    ret = pthread_create(&thread[n],NULL,testfunc,(void *)cpu[n]);//Create thread N, pass in parameter N
    if(ret != 0)
    {
      printf("Thread %d create failed\n",n);
      exit(ret);
    }
  }
  printf("Create %d threads success!\n",threadNum);

  printf("pid=%d\n", getpid());

  /* Recycling thread */
  for(n = 1;n < threadNum;n++)
  {
    int ret;
    ret = pthread_join(thread[n],NULL);
    if(!ret)
    {
      printf("Thread %d joined success\n",n);
    }else{
       printf("Thread %d joined failed\n",n);
    }
  }

  return 0;
}
