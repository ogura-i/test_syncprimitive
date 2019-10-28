#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define cpu_relax() __asm__("rep;nop":::"memory")

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#define LOCK_PREFIX_HERE \
                ".pushsection .smp_locks,\"a\"\n"       \
                ".balign 4\n"                           \
                ".long 671f - .\n" /* offset */         \
                ".popsection\n"                         \
                "671:"

#define LOCK_PREFIX LOCK_PREFIX_HERE "\n\tlock; "

#define __xadd(ptr, arg, lock)                                   \
         ({                                                      \
                 __typeof__ (*(ptr)) __ret = (arg);              \
                 asm volatile (lock "xaddw %w0, %1\n"            \
                               : "+q" (__ret), "+m" (*(ptr))     \
                               : : "memory", "cc");              \
                 __ret;                                          \
         })

#define my_xadd(ptr, inc)  __xadd((ptr), (inc), LOCK_PREFIX)

#define __add(ptr, inc, lock)                                    \
         ({                                                      \
                 __typeof__ (*(ptr)) __ret = (inc);              \
                 asm volatile (lock "addb %b1, %0\n"             \
                               : "+m" (*(ptr)) : "qi" (inc)      \
                               : "memory", "cc");                \
                 __ret;                                          \
         })

#define my_add(ptr, inc) __add((ptr), (inc), LOCK_PREFIX)

/*
 * define type of value
 */
typedef unsigned char ticket_t;
typedef struct my_spinlock_t {
        ticket_t head, tail;
} my_spinlock_t;

/*
 * define global values
 */
my_spinlock_t *lock;
int a = 0;
int n = 100000;
int flag;

/*
 * initialize my_spinlock_t
 */
void init_lock(){
    lock = malloc(sizeof(my_spinlock_t));
    memset(lock, 0, sizeof(my_spinlock_t));
}

/*
 * function of my_spin_lock
 * method is ticket spinlock
 */
int my_spin_lock(){

    register my_spinlock_t inc = {.tail = 1};

    inc = my_xadd(lock, inc);

    if(inc.head == inc.tail){
        return 0;
    }

    if(flag){
        for(;;){
            if (ACCESS_ONCE(lock->head) == inc.tail){
                return 0;
            }
            cpu_relax();
        }
    }
}

/*
 * function of my_spin_unlock
 */
void my_spin_unlock(){
    my_add(&lock->head, 1);
}

/*
 * function of increment
 */
void increment(int x){
    int i;

    printf("start thread%d\n", x);

    for(i = 0; i < n; i++){
        my_spin_lock();
        int next = a + 1;
        int now = a;
        a = next;
        my_spin_unlock();
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

int main(int argc, char *argv[]){
    int i, result;
    int x = 1, y = 2, z = 3;

    cpu_set_t cpu_set;
    pthread_t pthread[3];

    /*
     * init my_spinlock_t
     */
    init_lock();

    /*
     * set flag
     * flag use by doing lock or not
     */
    flag = atoi(argv[1]);

    // create
    pthread_create(&pthread[0], NULL, &func_thread, &x);
    pthread_create(&pthread[1], NULL, &func_thread, &y);
    pthread_create(&pthread[2], NULL, &func_thread, &z);

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

    printf("head: %hhu, tail: %hhu\n", lock->head, lock->tail);
    printf("a=%d\n", a);
    return 0;
}
