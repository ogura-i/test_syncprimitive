#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/page.h>       // define __va()
#include <linux/my_spinlock_k.h>

MODULE_LICENSE("Dual BSD/GPL");

#define WAIT_FLAG             0x1000010
#define GLOBAL_VALUE          0x1000100

unsigned int *a = (int *)__va(GLOBAL_VALUE);
unsigned int *wait_flag = (int *)__va(WAIT_FLAG);

int increment(void){
    int i;
    unsigned int t = 1;

    for(i = 0; i < 100000; i++){
        my_xadd(ACCESS_ONCE(a), t);
    }
    return 0;
}

static int hello_init(void){

    *wait_flag = 1;
    increment();

    printk(KERN_ALERT "a = %d\n", *a);

    return 0;

}

static void hello_exit(void){
    printk(KERN_ALERT "Goodbye cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
