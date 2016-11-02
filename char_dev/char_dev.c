/**
	A simple char device - Angelo Poerio <angelo.poerio@gmail.com>
        It is very far from a real char device. If you want to learn more, download a copy of the Linux kernel
        and then look at drivers/char!
        How to use (as root):
        make
        insmod char_dev.ko
        cat /proc/devices|grep char
        --> if everything worked you should get something like: 246 char_dev (number could change on your system)
        mknod /dev/test_dev c 246 0
        chmod 664 /dev/test
	echo "yooooo" > /dev/test
        dmesg|grep "from the"
        --> if everything worked you should get: char_dev: received 7 characters from the user space
        exec 4</dev/test
        read -u 4 buf
        dmesg|grep "from the"
        -- if everything worked you should get: char_dev: sent bytes to user space              
*/

#include <linux/init.h>           
#include <linux/module.h>        
#include <linux/device.h>         
#include <linux/kernel.h>        
#include <linux/fs.h>             
#include <asm/uaccess.h>          
#define  DEVICE_NAME "char_dev"    
#define  CLASS_NAME  "char_dev"        

MODULE_LICENSE("GPL");            
MODULE_AUTHOR("Angelo Poerio");    
MODULE_DESCRIPTION("A simple Linux char driver for learning purpose!");  
MODULE_VERSION("0.1");            
static int    majorNumber;                  
static char   message[5] = "test\0";                    
static int    opened_times = 0;              
static struct class*  charClass  = NULL; 
static struct device* charDevice = NULL; 
static DEFINE_MUTEX(char_dev_mutex);

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init chardev_init(void){

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "char_dev failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "char_dev: registered correctly with major number %d\n", majorNumber);

   charClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charClass)){                
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charClass);          
   }
   printk(KERN_INFO "char_dev: device class registered correctly\n");

   charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDevice)){               
      class_destroy(charClass);           
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDevice);
   }
   printk(KERN_INFO "char_dev: device class created correctly\n"); 
   return 0;
}


static void __exit chardev_exit(void){
   device_destroy(charClass, MKDEV(majorNumber, 0));     
   class_unregister(charClass);                          
   class_destroy(charClass);                             
   unregister_chrdev(majorNumber, DEVICE_NAME);             
   printk(KERN_INFO "char_dev unloaded\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
   mutex_lock(&char_dev_mutex); /* avoid race conditions */
   opened_times++;
   mutex_unlock(&char_dev_mutex);
   printk(KERN_INFO "char_dev: opened %d times\n", opened_times);
   return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   printk(KERN_INFO "char_dev: sent bytes to user space\n");
   return copy_to_user(buffer, message, 5);
}


static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   printk(KERN_INFO "char_dev: received %zu characters from the user space\n", len);
   return len;
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "char_dev successfully closed\n");
   return 0;
}

module_init(chardev_init);
module_exit(chardev_exit);
