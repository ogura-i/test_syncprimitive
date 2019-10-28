#include <linux/init.h>
#include <asm/page.h>       // define __va()

#define MINT_SHARED_MEM_START 0x1000000
#define FLAG                  0x1000020

#define _GNU_SOURCE

#define cpu_relax() __asm__("rep;nop":::"memory")

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

#define LOCK_PREFIX_HERE \
                ".pushsection .smp_locks,\"a\"\n"       \
                ".balign 4\n"                           \
                ".long 671f - .\n" /* offset */         \
                ".popsection\n"                         \
                "671:"

#define LOCK_PREFIX LOCK_PREFIX_HERE "\n\tlock; "

#define __my_xadd(ptr, arg, lock)                                   \
         ({                                                      \
                 __typeof__ (*(ptr)) __ret = (arg);              \
                 asm volatile (lock "xaddw %w0, %1\n"            \
                               : "+q" (__ret), "+m" (*(ptr))     \
                               : : "memory", "cc");              \
                 __ret;                                          \
         })

#define my_xadd(ptr, inc)  __my_xadd((ptr), (inc), LOCK_PREFIX)

#define __my_add(ptr, inc, lock)                                    \
         ({                                                      \
                 __typeof__ (*(ptr)) __ret = (inc);              \
                 asm volatile (lock "addb %b1, %0\n"             \
                               : "+m" (*(ptr)) : "qi" (inc)      \
                               : "memory", "cc");                \
                 __ret;                                          \
         })

#define my_add(ptr, inc) __my_add((ptr), (inc), LOCK_PREFIX)

typedef unsigned char ticket_t;

typedef struct my_spinlock_t {
        ticket_t head, tail;
} my_spinlock_t;

my_spinlock_t *lock = (my_spinlock_t *)__va(MINT_SHARED_MEM_START);
unsigned int *flag = (int *)__va(FLAG);

void my_spinlock_init(int x){
    memset(lock, 0, sizeof(my_spinlock_t));
    *flag = x;

}

int my_spin_lock(void){

    register my_spinlock_t inc = {.tail = 1};

    inc = my_xadd(lock, inc);

    if(inc.head == inc.tail){
        return 0;
    }

    if(*flag){
        for(;;){
            if (ACCESS_ONCE(lock->head) == inc.tail){
                return 0;
            }
            cpu_relax();
        }
    }
    return 0;
}

void my_spin_unlock(void){
    my_add(&lock->head, 1);
}

