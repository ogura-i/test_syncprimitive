#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/page.h>       // define __va()
#include <linux/my_spinlock_k.h>

MODULE_LICENSE("Dual BSD/GPL");

/*
 * define sharedmem address
 */
#define WAIT_FLAG             0x1000010
#define GLOBAL_VALUE          0x1000100

/*
 * define global value
 */
unsigned int *a = (unsigned int *)__va(GLOBAL_VALUE);
unsigned int *wait_flag = (unsigned int *)__va(WAIT_FLAG);

/*
 * function of flag_checker
 * this is wait following device driver
 */
int flag_checker(void){
    for(;;){
        if(ACCESS_ONCE(*wait_flag) == 1){
            break;
        }
    }
    return 0;
}

/*
 * function of increment
 */
int increment(void){
    int i;
    unsigned int t = 1;

    for(i = 0; i < 100000; i++){
        my_xadd(ACCESS_ONCE(a), t);
    }
    return 0;
}


static int increment_inter_kernel_init(void){

    *a = 0;
    *wait_flag = 0;

    flag_checker();
    increment();
    
    msleep(500);

    printk(KERN_ALERT "a = %d\n", *a);

    return 0;
}

static void increment_inter_kernel_exit(void){
    printk(KERN_ALERT "Goodbye cruel world\n");
}

module_init(increment_inter_kernel_init);
module_exit(increment_inter_kernel_exit);
