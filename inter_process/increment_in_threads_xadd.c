#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "my_spinlock_u.h"

/*
 * define global value
 */
unsigned int *sum;
int loop_upper = 100000;

/*
 * function of increment
 */
void increment(int x){
    int i;
    register unsigned int t = 1;

    printf("start thread%d\n", x);

    for(i = 0; i < loop_upper; i++){
        my_xadd(ACCESS_ONCE(sum), t);
    }
}

/*
 * child threads function
 */
void *func_thread(void *p){
    int result;
    cpu_set_t cpu_set;

    // init set_cpu
    CPU_ZERO(&cpu_set);
    CPU_SET(*(int*)p, &cpu_set);

    // bind child thread to the coreX
    result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set);
    if(result != 0){
        printf("core affinity is faild in thread%d\n", *(int*)p);
    }

    // call function of increment
    increment(*(int*)p);
}

int main(){
    int i, result;
    int x = 0, y = 1, z = 2, w = 3;


    cpu_set_t cpu_set;
    pthread_t pthread[3];

    /*
     * init global value "a"
     */
    sum = malloc(sizeof(unsigned int));
    memset(sum, 0, sizeof(unsigned int));

    // create
    pthread_create(&pthread[0], NULL, &func_thread, &y);
    pthread_create(&pthread[1], NULL, &func_thread, &z);
    pthread_create(&pthread[2], NULL, &func_thread, &w);

    // init cpu_set
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

    // bind main thread to the core0
    result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set);
    if(result != 0){
        printf("core affinity is faild in thread0\n");
    }

    // call function of increment
    increment(0);

    // join child threads
    pthread_join(pthread[0], NULL);
    pthread_join(pthread[1], NULL);
    pthread_join(pthread[2], NULL);

    printf("sum = %d\n", *sum);
    return 0;
}
