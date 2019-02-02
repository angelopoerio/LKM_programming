#include <linux/module.h>       
#include <linux/kernel.h>       
#include <linux/sched.h>        

void procs_info_list(void)
{
        struct task_struct* task_list;
        for_each_process(task_list) {
                pr_info("%s [%d]\n", task_list->comm, task_list->pid);
        }
}

int init_module(void)
{
        printk(KERN_INFO "procs_list INIT\n");

        procs_info_list();

        return 0;
}

void cleanup_module(void)
{
        printk(KERN_INFO "procs_list CLEANUP\n");
}

MODULE_LICENSE("MIT");
