#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

// 初始化函数
static int hello_init(void)
{
    struct task_struct *p;  //Linux内核的进程控制块是task_struct结构体，所有运行在系统中的进程都以task_struct链表的形式存在内核中
    printk(KERN_ALERT"            名称\t进程号\t状态  \t优先级\t父进程号\t");
    for_each_process(p)  //for_each_process是一个宏，在sched.h里面定义: 是从init_task开始遍历系统所有进程，init_task是进程结构链表头。
    {
        if(p->mm == NULL){ //对于内核线程，mm为NULL
            printk(KERN_ALERT"%16s\t%-6d\t%-6ld\t%-6d\t%-6d\n",p->comm,p->pid, p->state,p->normal_prio,p->parent->pid);
        }
    }
    return 0;
}
// 清理函数
static void hello_exit(void)
{
    printk(KERN_ALERT"goodbye!\n");
}

// 函数注册
module_init(hello_init);  
module_exit(hello_exit);  

// 模块许可申明
MODULE_LICENSE("GPL");  
