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
#include <linux/device.h> //to automatically make devices

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
==============================================================================*/

//major # is the device # for this driver, minor # is the variant of major #
static int major_number; 
static struct class *device_class = NULL;
static struct device *device_device = NULL;


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
    printk(KERN_INFO "virtual_device: Device opened\n");
    return 0;
}

//called when someone closes our device
static int device_release(struct inode *inodePath, struct file *filePath) {
    printk(KERN_INFO "virtual_device: Device closed\n");
    return 0;
}

//driver init
static int __init virtual_device_init(void) {
    printk(KERN_INFO "virtual_device: Driver loaded\n");

    //register major_number for character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "virtual_device: Failed to register major number\n");
        return major_number;
    }

    // we keep it extra verbose here
    printk(KERN_INFO "virtual_device: Registered character device with major number %d\n", major_number);
    
    //create device class
    device_class = class_create("virtual_device_class");
    if (IS_ERR(device_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "virtual_device: failed to create device_class\n");
        return PTR_ERR(device_class);
    }

    //create device file
    device_device = device_create(device_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(device_device)) {
        class_destroy(device_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "virtual device: failed to create device_device\n");
        return PTR_ERR(device_device);
    }
    
    printk(KERN_INFO "virtual_device: Created device at /dev/%s\n", DEVICE_NAME);

    return 0;
}

//driver exit
static void __exit virtual_device_exit(void) {
    //destroy and unregister the character device and class
    device_destroy(device_class, MKDEV(major_number, 0));
    class_destroy(device_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "virtual_device: Driver unloaded\n");

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