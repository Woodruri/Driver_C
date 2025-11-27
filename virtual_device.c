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
#include <linux/uaccess.h> // to add user access to the device

/*==============================================================================
defines
==============================================================================*/


//Makes things cleaner, no magic names here, so sir
#define DEVICE_NAME "virtual_device"

//control register bits for use in the hardware_registers struct
#define CTRL_ENABLE (1 << 0) // enable device
#define CTRL_RESET (1 << 1) // reset device

//status register bits for use in the hardware_registers struct
#define STATUS_READY (1 << 0) // device ready
#define STATUS_BUSY (1 << 1) // device busy
#define STATUS_ERROR (1 << 2) // error occured in device


/*==============================================================================
prototypes
==============================================================================*/

//for file operations
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

//for user access
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);


/*==============================================================================
global vars
==============================================================================*/

//major # is the device # for this driver, minor # is the variant of major #
static int major_number; 

//the device global vars
static struct class *device_class = NULL;
static struct device *device_device = NULL;

//what simulates the hardware in a real physical device
static struct hardware_registers {
    uint32_t control; //control register, where commands are written to
    uint32_t status; //status register, hardware updates this
    uint32_t data; //data register, what handles data read/writes
    uint32_t counter; //counter register, auto increments to simulate a hardware counter
};

// global variable for the above struct
static struct hardware_registers *hw_regs = NULL;

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
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

//called when userspace reads from our device
static ssize_t device_read(struct file *filePath, char __user *buffer, size_t len, loff_t *offest) {
    //copy value to user buffer

    uint32_t value;

    value = hw_regs->status;    

    printk(KERN_INFO "virtual_deivce: Read status register: 0x%08x\n", value);

    //copy to userspace
    if (copy_to_user(buffer, &value, sizeof(value))) {
        return -EFAULT; // error for invalid user space address being sent
    }

    
    return sizeof(value);
}

//called when userspace writes to our device
static ssize_t device_write(struct file *filePath, const char __user *buffer, 
                            size_t len, loff_t *offset) {
    //write from userspace to our buffer
    uint32_t value;

    //ensure that the len is not greater than value can handle
    if (len < sizeof(value)) {
        return -EINVAL; // if the size of device value exceeds len, error with invalid argument
    }

    //copy from userspace to our device
    if (copy_from_user(&value, buffer, sizeof(value))) {
        return -EINVAL; // same as above
    }

    //write value to control
    hw_regs->control = value;
    printk(KERN_INFO "virtual_device: wrote control register: 0x%08x\n", value);

    //being verbose for clarity cuz bitwise operators get tricky
    //This simulates recieving a command and becoming busy
    if ((value & CTRL_ENABLE) != 0) {
        hw_regs->status = STATUS_BUSY;
        printk(KERN_INFO "virtual_device: Device enabled, now BUSY\n");
    }
    //This simulates recieving a reset command
    if ((value & CTRL_RESET) != 0) {
        hw_regs->counter = 0;
        hw_regs->status = STATUS_READY;
        printk(KERN_INFO "virtual_device: Device reset, counter cleared and set to READY\n");
    }
    
    return sizeof(value);
}

//driver init
static int __init virtual_device_init(void) {
    printk(KERN_INFO "virtual_device: Initializing...\n");

    //allocate memory for hardware registers (GFP_KERNEL means Get Free Pages in Kernel lol)
    hw_regs = kmalloc(sizeof(struct hardware_registers), GFP_KERNEL);
    if (!hw_regs) {
        printk(KERN_ERR "virtual_device: Failed to allocate hardware registers\n");
        return -ENOMEM; //This is an error if there is no memory available
    }

    //initialize the hardware registers (basically hardware reset state)
    hw_regs->control = 0;
    hw_regs->counter = STATUS_READY;
    hw_regs->data = 0;
    hw_regs->counter=0;

    printk(KERN_INFO "virtual_device: Hardware registers initialized properly\n");

    //register major_number for character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    //deallocate the allocated memory in the kernel if we can't register the character device
    if (major_number < 0) {
        kfree(hw_regs);
        printk(KERN_ALERT "virtual_device: Failed to register major number\n");
        return major_number;
    }

    // we keep it extra verbose here
    printk(KERN_INFO "virtual_device: Registered character device with major number %d\n", major_number);
    
    //create device class
    device_class = class_create("virtual_device_class");
    //deallocate the allocated stuff in the kernel if we can't register the device class
    if (IS_ERR(device_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        kfree(hw_regs);
        printk(KERN_ERR "virtual_device: failed to create device_class\n");
        return PTR_ERR(device_class);
    }

    //create device file
    device_device = device_create(device_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    //deallocate the allocated stuff in the kernel if we can't register the device file
    if (IS_ERR(device_device)) {
        class_destroy(device_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        kfree(hw_regs);
        printk(KERN_ERR "virtual device: failed to create device_device\n");
        return PTR_ERR(device_device);
    }

    printk(KERN_INFO "virtual_device: Driver loaded\n");
    printk(KERN_INFO "virtual_device: Created device at /dev/%s\n", DEVICE_NAME);

    return 0;
}

//driver exit
static void __exit virtual_device_exit(void) {
    //destroy and unregister the character device, class, file, and allocated memory
    device_destroy(device_class, MKDEV(major_number, 0));
    class_destroy(device_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    kfree(hw_regs);

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