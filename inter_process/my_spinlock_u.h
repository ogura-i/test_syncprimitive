#define _GNU_SOURCE
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
int flag;

/*
 * initialize my_spinlock_t
 */
void my_spinlock_init(int x){
    lock = malloc(sizeof(my_spinlock_t));
    memset(lock, 0, sizeof(my_spinlock_t));
    flag = x;
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

