#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");

static int kernel_version[3];
static int count;
static int timer;

static dev_t dev_num;

module_param_array(kernel_version, int, &count, 0);
module_param(timer, int, 0);

static int __init my_init(void)
{
    int input_version;
    int ret;

    if (count < 3) {
        return -EINVAL;
    }
    int actual_major = (LINUX_VERSION_CODE >> 16) & 0xFF;
    int actual_minor = (LINUX_VERSION_CODE >> 8) & 0xFF;
    int actual_patch = LINUX_VERSION_CODE & 0xFF;
    input_version = KERNEL_VERSION(kernel_version[0], kernel_version[1], kernel_version[2]);

    if (input_version != LINUX_VERSION_CODE) {
        printk("Given Kernel Version does not match with current Kernel Version\n");
        printk("You gave: %d.%d.%d\n", kernel_version[0], kernel_version[1], kernel_version[2]);
        printk("Expected version : %d.%d.%d\n", actual_major, actual_minor, actual_patch);
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&dev_num, 0, 1, "mydevice");
    if (ret < 0) {
        printk("Failed to allocate char device region\n");
        return ret;
    }

    printk("Kernel version matched\n");

    printk("Major number: %d\n", MAJOR(dev_num));
    printk("Minor number: %d\n", MINOR(dev_num));
    printk("Timer: %d seconds\n", timer);

    return 0;
}

static void __exit my_exit(void)
{
    unregister_chrdev_region(dev_num, 1);
    printk("Module removed\n");
}

module_init(my_init);
module_exit(my_exit);
