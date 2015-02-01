#include <linux/sched.h>
#include <linux/file.h>
#include <linux/version.h>
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/proc_fs.h>  /* Necessary because we use proc fs */
#include <asm/uaccess.h>    /* for copy_*_user */
#include <linux/fs.h>

//#include <linux/cred.h>

#define PROC_ENTRY_FILENAME     "buffer2k"
#define PROCFS_MAX_SIZE 2048


static char procfs_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *Our_Proc_File;

static ssize_t procfs_read( struct file * file,
                            char    *buffer,
                            size_t  length,
                            loff_t  *offset)

{

    static int finished =   0;

    if( finished ){
        printk(KERN_INFO "procfs_read: END");
        finished = 0;
        return 0;
    }

    finished = 1;

    if ( copy_to_user(buffer, procfs_buffer, procfs_buffer_size)) {
        return -EFAULT;
    }

    printk(KERN_INFO "procfs_read: read %lu bytes \n", procfs_buffer_size);
    return procfs_buffer_size;
}

static ssize_t procfs_write(struct  file    *file,
                            const   char    *buffer,
                            size_t  len,
                            loff_t  *offset)
{
    if ( len > PROCFS_MAX_SIZE ) {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    } else {
        procfs_buffer_size = len;
    }

    if ( copy_from_user( procfs_buffer, buffer, procfs_buffer_size)){
        printk(KERN_ALERT  "Debug: here after copy he result. ");
        return -EFAULT;
    }

    printk( KERN_INFO "procfs_write: write %lu bytes \n", procfs_buffer_size);

    return procfs_buffer_size;
}

static int module_permission( struct inode  *inode,
                             int    op
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,28)
                             , struct nameidata   *foo
#endif
)
{

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,28)
#define current_euid()  current->euid
#endif

    printk( KERN_ALERT "Debug: op is %d \n", op);
   // printk( KERN_ALERT "debug: get current user %d. \n", current_euid());
    if  (  op == 36 || op == 4 || (   op == 2  && current_euid() == 0))
        return 0;

    return -EACCES;
}

int procfs_open ( struct    inode *inode,
                    struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

int procfs_close(struct inode   *inode,
                struct  file    *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static struct file_operations   File_Ops_Proc_FIle = {
    .read   =   procfs_read,
    .write  =   procfs_write,
    .open   =   procfs_open,
    .release    =   procfs_close,
};


static struct inode_operations Inode_Ops_Proc_File = {
    .permission =   module_permission,
};

int init_module(){
    Our_Proc_File = create_proc_entry(PROC_ENTRY_FILENAME, 0666, NULL);

    if ( Our_Proc_File == NULL){
        printk(KERN_ALERT "Error: could not initialize /proc/%s\n", PROC_ENTRY_FILENAME);
        return -ENOMEM;
    }

    Our_Proc_File->proc_iops    =   &Inode_Ops_Proc_File;
    Our_Proc_File->write_proc   =   &File_Ops_Proc_FIle;
    Our_Proc_File->mode          =   S_IFREG | S_IRUGO   | S_IWUSR;
    Our_Proc_File->uid          =   0;
    Our_Proc_File->gid          =   0;
    Our_Proc_File->size         =   80;

    printk( KERN_INFO   "/proc/%s created \n", PROC_ENTRY_FILENAME);

    return 0;
}

void cleanup_module()
{
    remove_proc_entry( PROC_ENTRY_FILENAME, NULL);
    printk( KERN_INFO "/proc/%s removed \n", PROC_ENTRY_FILENAME);
}
