#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/string.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");

static int kernel_version[2];
static int count;
static int timer;

module_param_array(kernel_version, int, &count, 0);
module_param(timer, int, 0);

static dev_t dev_num;
static struct cdev my_cdev;
static wait_queue_head_t wq;

static bool read_done = false;
static bool write_done = false;
static bool order_correct = false;
static bool time_expired = false;
static unsigned long timeout_jiffies;
static char username[100];

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    long time_left;
    long ret;
    size_t bytes_to_copy;

    if (*off > 0)
        return 0;

    time_left = timeout_jiffies - jiffies;
    if (time_left <= 0) {
        time_expired = true;
        printk("Timer expired.\n");
        return -ETIME;
    }

    read_done = true;

    ret = wait_event_interruptible_timeout(wq, write_done, time_left);

    if (ret == 0) {
        time_expired = true;
        printk("Timer expired.\n");
        return -ETIME; 
    } else if (ret < 0) {
        return -ERESTARTSYS;
    }

    bytes_to_copy = strlen(username);
    if (len < bytes_to_copy)
        bytes_to_copy = len;

    if (copy_to_user(buf, username, bytes_to_copy)) {
        return -EFAULT;
    }

    *off += bytes_to_copy;
    return bytes_to_copy;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    long time_left;
    size_t bytes_to_copy;

    time_left = timeout_jiffies - jiffies;
    if (time_left <= 0) {
        time_expired = true;
        printk("Timer expired.\n");
        return -ETIME;
    }

    if (!read_done) {
        order_correct = false;
        printk("Order violation! Write called before Read.\n");
    } else if (!write_done) {
        order_correct = true;
    }

    bytes_to_copy = len;
    if(sizeof(username) - 1 < bytes_to_copy)
        bytes_to_copy = sizeof(username) - 1;

    if (copy_from_user(username, buf, bytes_to_copy)) {
        return -EFAULT;
    }

    username[bytes_to_copy] = '\0';

    write_done = true;

    wake_up_interruptible(&wq);

    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
    int actual_major = (LINUX_VERSION_CODE >> 16) & 0xFF;
    int actual_minor = (LINUX_VERSION_CODE >> 8) & 0xFF;
    int ret;

    if (count < 2) {
        printk("Insufficient arguments\n");
        return -EINVAL;
    }

    if (kernel_version[0] != actual_major || kernel_version[1] != actual_minor) {
        printk("Kernel Version mismatch!\n");
        printk("You gave: %d.%d\n", kernel_version[0], kernel_version[1]);
        printk("Expected : %d.%d\n", actual_major, actual_minor);
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&dev_num, 0, 1, "mydevice");
    if (ret < 0) {
        printk("Failed to allocate char device region\n");
        return ret;
    }

    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    init_waitqueue_head(&wq);

    timeout_jiffies = jiffies + (timer * HZ);

    printk("Kernel version matched.\n");
    printk("Major number: %d\n", MAJOR(dev_num));
    printk("Minor number: %d\n", MINOR(dev_num));
    printk("Timer started for: %d seconds\n", timer);

    return 0;
}

static void __exit my_exit(void)
{
    if (read_done && write_done && order_correct && !time_expired) {
        printk("Successfully completed the actions within time.\n");
        printk("Username recorded: %s\n", username);
    } else {
        printk("Failure - Actions were not completed in order within the given time.\n");
    }

    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk("Module removed\n");
}

module_init(my_init);
module_exit(my_exit);