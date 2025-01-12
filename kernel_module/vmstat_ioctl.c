#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>

#define DEVICE_NAME "vmstat_ioctl"
#define IOCTL_GET_VMSTAT _IOR('v', 1, char *)

int count_running_processes(void);

int count_running_processes(void) {
    struct task_struct *task;
    int running = 0;

    
    for_each_process(task) {
        if (task_is_running(task)) {
            running++;
        }
    }

    
    return running;
}

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
    char buffer[1024];
    int len;
    struct file *f;
    char buf[1024];
    loff_t pos = 0;
    unsigned long cachedram = 0;
    unsigned long interrupts = 0, context_switches = 0;
    unsigned long user_time, sys_time, idle_time;

    if (cmd == IOCTL_GET_VMSTAT) {
        si_meminfo(&si);
        int running = count_running_processes();

        // Получение информации о памяти
        f = filp_open("/proc/meminfo", O_RDONLY, 0);
        if (IS_ERR(f)) {
            printk(KERN_ERR "Error opening /proc/meminfo\n");
            return PTR_ERR(f);
        }

        vfs_read(f, buf, sizeof(buf), &pos);
        filp_close(f, NULL);
        
        if (sscanf(buf, "Cached: %lu kB", &cachedram) != 1) {
            cachedram = 0;
        }

        // Получение статистики с процессора
        f = filp_open("/proc/stat", O_RDONLY, 0);
        if (IS_ERR(f)) {
            printk(KERN_ERR "Error opening /proc/stat\n");
            return PTR_ERR(f);
        }

        vfs_read(f, buf, sizeof(buf), &pos);
        filp_close(f, NULL);

        // Считывание метрик CPU 
        sscanf(buf, "cpu  %lu %lu %lu %lu", &user_time, &sys_time, &idle_time, &interrupts);

        // Вычисление контекстных переключений
        f = filp_open("/proc/stat", O_RDONLY, 0);
        if (IS_ERR(f)) {
            printk(KERN_ERR "Error opening /proc/stat for context switches\n");
            return PTR_ERR(f);
        }

        vfs_read(f, buf, sizeof(buf), &pos);
        filp_close(f, NULL);

        // Извлекаем количество переключений контекста
        sscanf(buf, "ctxt %lu", &context_switches);

        // Форматируем вывод
        len = snprintf(buffer, sizeof(buffer),
            "VMStat Information:\n"
             "procs---memory------swap--------io------system-------cpu------\n"
            " r b swpd free buff cache si so bi bo in cs us sy id wa st gu\n"
            " %ld %ld %lu %ld %ld %ld %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
            running, 0, // Количество процессов в состоянии r и b
            si.totalswap << (PAGE_SHIFT - 10),
            si.freeram << (PAGE_SHIFT - 10),
            si.bufferram << (PAGE_SHIFT - 10),
            cachedram,
            interrupts, // Количество прерываний
            context_switches, // Количество переключений контекста
            user_time, sys_time, idle_time, 0, 0, 0); // Время процессора

        // Отправка данных пользователю
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


