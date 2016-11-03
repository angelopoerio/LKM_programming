/*
	A char driver blocking processes in wait queue - Angelo Poerio <angelo.poerio@gmail.com>
        insmod char_dev_w.ko
        then in a terminal:
        cat /dev/charw0 (process will stay blocked)
        ps -Ao pid,state,cmd|grep char
        --> 7375 S cat /dev/charw0 (state 'S', interruptible process)
        You can unblock the process issuing this command: echo 'e' > /dev/charw0 
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define MIN 1

static char flag = 'd';
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static DECLARE_WAIT_QUEUE_HEAD(wq);

int open(struct inode *inode, struct file *filp)
{
	return 0;
}

int release(struct inode *inode, struct file *filp) 
{
	return 0;
}

ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp) 
{
	printk(KERN_INFO "char_dev_w: %d process waiting\n", current->pid);
	wait_event_interruptible(wq, flag == 'e');
	flag = 'd';
	printk(KERN_INFO "char_dev_w: %d process woke up\n", current->pid);
	return 0;
}

ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp) 
{   
	if (copy_from_user(&flag, buff, 1))
	{
		return -EFAULT;
	}
	printk(KERN_INFO "char_dev_w: wrote %c", flag);
	wake_up_interruptible(&wq);
	return count;
}

struct file_operations pra_fops = {
	read:        read,
	write:       write,
	open:        open,
	release:     release
};

int wq_init (void)
{
	int ret;
	struct device *dev_ret;

	if ((ret = alloc_chrdev_region(&dev, 0, MIN, "char_dev_w")) < 0)
	{
		return ret;
	}

	cdev_init(&c_dev, &pra_fops);

	if ((ret = cdev_add(&c_dev, dev, MIN)) < 0)
	{
		unregister_chrdev_region(dev, MIN);
		return ret;
	}

	if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv_w")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MIN);
		return PTR_ERR(cl);
	}
	if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "charw%d", 0)))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MIN);
		return PTR_ERR(dev_ret);
	}
	return 0;
}

void wq_cleanup(void)
{
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MIN);
}

module_init(wq_init);
module_exit(wq_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Angelo Poerio");
MODULE_DESCRIPTION("Char driver blocking processes in wait queue");
