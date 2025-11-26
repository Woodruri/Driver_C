/*
Where the virtual device driver is built
Riley Woodruff Nov 26, 2025
*/

/*==============================================================================
includes
==============================================================================*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h> //character device operations

/*==============================================================================
defines
==============================================================================*/


#define DEVICE_NAME "virtual_device"


/*==============================================================================
prototypes
==============================================================================*/

//for file operations
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);


/*==============================================================================
global vars
/*==============================================================================*/

//major # is the device # for this driver, minor # is the variant of major #
static int major_number; 


/*==============================================================================
structs
==============================================================================*/

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
};


/*==============================================================================
driver code
==============================================================================*/

//called when someone opens our device
static int device_open(struct inode *inodePath, struct file *filePath) {
    printk(KERN_INFO, "virtual_device: Device opened\n");
    return 0;
}

//called when someone closes our device
static int device_release(struct inode *inodePath, struct file *filePath) {
    printk(KERN_INFO, "virtual_device: Device closed\n");
    return 0;
}

//driver init
static int __init virtual_device_init(void) {
    printk(KERN_INFO, "virtual_device: Driver loaded\n");
    return 0;
}

//driver exit
static void __exit virtual_device_exit(void) {
    printk(KERN_INFO, "virtual_device: Driver unloaded\n");
}


/*==============================================================================
driver init/exit
==============================================================================*/

module_init(virtual_device_init);
module_exit(virtual_device_exit);


/*==============================================================================
driver info
==============================================================================*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Riley Woodruff");
MODULE_DESCRIPTION("Learning to write a device driver + use volatile keyword");