/*
Where the virtual device driver is built
Riley Woodruff Nov 26, 2025
*/


//includes
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>



static int __init virtual_device_init(void) {
    printk(KERN_INFO, "virtual_device: Driver loaded\n");
    return 0;
}

static void __exit virtual_device_exit(void) {
    printk(KERN_INFO, "virtual_device: Driver unloaded\n");
}

module_init(virtual_device_init);
module_exit(virtual_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Riley Woodruff");
MODULE_DESCRIPTION("Learning to write a device driver + use volatile");