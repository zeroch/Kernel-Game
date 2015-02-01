#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/unistd.h>

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

    /* return original_call(filename, flags, mode); */
/* } */

int procfile_read(  char *buffer,
        char **buffer_location,
        off_t offset,
        int buffer_length,
        int *eof,
        void *data)
{

    int ret;
    /* printk(KERN_INFO "procfile_read(/proc/%s) called \n", procfs_name); */

    if ( offset > 0) {
        ret = 0;
    } else {

        char *read_data = (char *)data;
       if (!memcpy(buffer, read_data, procfs_buffer_size))
       /* if ( copy_to_user(read_data, buffer, procfs_buffer_size) ) */
            return -EFAULT;
        ret = procfs_buffer_size;
    }

    return ret;
}

int procfs_write(   struct  file    *file,
        const char  *buffer,
        unsigned long count,
        void    *data)
{
    procfs_buffer_size = count;
    char * write_data = (char *)data;
    if ( procfs_buffer_size > PROCFS_MAX_SIZE) {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }

    if ( copy_from_user(write_data, buffer, procfs_buffer_size)) {
        return -EFAULT;
    }

    printk(KERN_ALERT "UID: %d", current_uid());
    return procfs_buffer_size;
}

void create_user_proc_entry(struct proc_dir_entry *new_proc,const char *filename, char * data)
{

    new_proc = create_proc_entry(filename, 0666, proc_parent);

    if ( new_proc == NULL) {
        remove_proc_entry(filename, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s \n", filename);
        return -ENOMEM;
    }

    new_proc->read_proc	= procfile_read;
    new_proc->write_proc   = procfs_write;
    //new_proc->owner		= THIS_MODULE;
    new_proc->mode		= S_IFREG | S_IRUGO;
    new_proc->uid		= 0;
    new_proc->gid		= 0;
    new_proc->size		= 37;
    new_proc->data      = (void *)data;
    printk(KERN_INFO "/proc/%s created \n", filename);

}

void create_new_proc_entry()
{
    proc_parent = proc_mkdir(username, NULL);
    if( proc_parent == NULL ){
        remove_proc_entry(username, NULL);
        printk( KERN_INFO "Error to create a folder ");
        return -ENOMEM;
    }

    create_user_proc_entry( Our_Proc_File, "game", procfs_buffer);
    create_user_proc_entry( proc_opponent, "oppoent", another_buffer);
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