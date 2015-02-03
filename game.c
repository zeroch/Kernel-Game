#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/unistd.h>
#include <linux/fs.h>

#define username  "zero"
#define PROCFS_MAX_SIZE 1024

static struct proc_dir_entry *proc_parent;
static struct proc_dir_entry *Our_Proc_File;
static struct proc_dir_entry *proc_opponent;
static int board[9];

static char procfs_buffer[PROCFS_MAX_SIZE];
static char another_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size = 0;
/* int uid; */
/* extern void *sys_call_table[]; */
/* asmlinkage int (*original_call)(const char *, int, int); */
/* asmlinkage int (*getuid_call)(); */

/* asmlinkage int my_sys_open(const char *filename, int flags, int mode) */
/* { */

    /* int i = 0; */
    /* char ch; */
    /* if( uid == getuid_call( )) { */
        /* printk("Openfile by %d", uid); */
        /* do { */
            /* get_user(ch,filename+i); */
            /* i++; */
            /* printk("%s", ch); */
        /* } while( ch != 0); */
    /* } */
    /* printk("\n"); */

    /*jjjeturn original_call(filename, flags, mode); */
/* } */

static ssize_t procfs_read( struct file *filp,
        char *buffer,
        size_t length,
        loff_t * offset)
{

    /* printk(KERN_INFO "procfs_read(/proc/%s) called \n", procfs_name); */

    static int finished = 0;

    if ( finished ) {
        printk(KERN_INFO "procfs_read: end \n");
        finished = 0;
        return 0;
    }

    finished = 1 ;

    if ( copy_to_user(buffer, procfs_buffer, procfs_buffer_size )) {
        return -EFAULT;
    }

    printk( KERN_INFO "PROCFS_READ READ %lu bytes \n", procfs_buffer_size);

    return procfs_buffer_size;
}

static ssize_t procfs_write(   struct  file    *file,
        const char  *buffer,
        size_t len,
        loff_t * off)
{
    if ( len > PROCFS_MAX_SIZE) {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }else {
        procfs_buffer_size = len;
    }

    if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
        return -EFAULT;
    }

    printk(KERN_ALERT "UID: %d", current_euid());
    return procfs_buffer_size;
}

static int module_permission (struct inode *inode, int op)
{
    if( op == 4 || (op == 2 && current_euid() == 0))
        return 0;


    return -EACCES;
}

int procfs_open( struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

int procfs_close(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static struct file_operations File_Ops_Proc_File = {
    .read   =   procfs_read,
    .write  =   procfs_write,
    .open   =   procfs_open,
    .release =  procfs_close,
};

static struct inode_operations Inode_Ops_Proc_FIle = {
    .permission =   module_permission,
};

int create_user_proc_entry( struct proc_dir_entry *new_proc,const char *filename, char * data)
{

    new_proc = proc_create(filename, 0644, proc_parent, &File_Ops_Proc_File);

    if ( new_proc == NULL) {
        remove_proc_entry(filename, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s \n", filename);
        return -ENOMEM;
    }

    /* new_proc->data = (void *)data; */

    /* if ( new_proc ) { */
    /* new_proc->proc_iops     = &Inode_Ops_Proc_FIle; */
    /* new_proc->proc_fops     = &File_Ops_Proc_File; */
    /* new_proc->owner		= THIS_MODULE; */
    /* new_proc->mode		= S_IFREG | S_IRUGO | S_IWUSR; */
    /* new_proc->uid		= 0; */
    /* new_proc->gid		= 0; */
    /* new_proc->size		= 80; */
    /* new_proc->data      = (void *)data; */
    /* } */
    printk(KERN_INFO "/proc/%s created \n", filename);
    return 0;
}

int create_new_proc_entry()
{
    proc_parent = proc_mkdir(username, NULL);
    if( proc_parent == NULL ){
        remove_proc_entry(username, NULL);
        printk( KERN_INFO "Error to create a folder ");
        return -ENOMEM;
    }

    create_user_proc_entry( Our_Proc_File, "game", procfs_buffer);
    create_user_proc_entry( proc_opponent, "oppoent", another_buffer);
    return 0;
}

int init_module()
{
    create_new_proc_entry();
    /* original_call = sys_call_table[__NR_open]; */
    /* sys_call_table[__NR_open] = my_sys_open; */

    /* printk("Current on UID: %d \n", uid); */

    /* getuid_call = sys_call_table[__NR_getuid]; */

    return 0;
}

void cleanup_module()
{
    /* if ( sys_call_table[__NR_open] != my_sys_open) { */
        /* printk(" looks fine"); */
    /* } */

    /* sys_call_table[__NR_open] = original_call; */

    remove_proc_entry("game", proc_parent);
    printk(KERN_INFO "/proc/%s removed\n", "game");
    remove_proc_entry("oppoent", proc_parent);
    printk(KERN_INFO "/proc/%s removed\n", "oppoent");
    remove_proc_entry(username, NULL);
    printk(KERN_INFO "/proc/%s removed\n", username);
}
