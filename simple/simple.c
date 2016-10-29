/*
	A simple start with linux kernel module programming!
        Compile: make
	Load module: insmod hello.ko
        Unload module: rmmod hello
        Check if everything worked: dmesg|grep -i simple
	Angelo Poerio - <angelo.poerio@gmail.com>
*/

#include <linux/init.h>             
#include <linux/module.h>           
#include <linux/kernel.h>           

MODULE_LICENSE("GPL");              
MODULE_AUTHOR("Angelo Poerio");      
MODULE_DESCRIPTION("A simple start");  
MODULE_VERSION("0.1");              


static int __init simple_init(void){
   printk(KERN_INFO "Simple start!\n");
   return 0;
}

static void __exit simple_exit(void){
   printk(KERN_INFO "Simple exit!\n");
}

module_init(simple_init);
module_exit(simple_exit);
