#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int Entering_module(void){
    printk("Entered the module\n");
    return 0;
}

static void Exiing_module(void){
    printk("Exiting the module\n");
}

module_init(Entering_module);
module_exit(Exiing_module);
