#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "vmstat_ioctl"
#define IOCTL_GET_VMSTAT _IOR('v', 1, char *)

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "vmstat_ioctl: device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "vmstat_ioctl: device closed\n");
    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct sysinfo si;
    char buffer[256];
    int len;

    if (cmd == IOCTL_GET_VMSTAT) {
        si_meminfo(&si);
        len = snprintf(buffer, sizeof(buffer),
            "Total RAM: %lu KB\nFree RAM: %lu KB\nUsed Swap: %lu KB\nFree Swap: %lu KB\n",
            si.totalram << (PAGE_SHIFT - 10),
            si.freeram << (PAGE_SHIFT - 10),
            si.totalswap << (PAGE_SHIFT - 10),
            si.freeswap << (PAGE_SHIFT - 10));

        if (copy_to_user((char *)arg, buffer, len)) {
            return -EFAULT;
        }

        return 0;
    }

    return -EINVAL;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

static int __init vmstat_ioctl_init(void) {
    int major;

    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "vmstat_ioctl: failed to register device\n");
        return major;
    }

    printk(KERN_INFO "vmstat_ioctl: device registered with major number %d\n", major);
    return 0;
}

static void __exit vmstat_ioctl_exit(void) {
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "vmstat_ioctl: device unregistered\n");
}

module_init(vmstat_ioctl_init);
module_exit(vmstat_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bermas Denis");
MODULE_DESCRIPTION("VMStat IOCTL Kernel Module");