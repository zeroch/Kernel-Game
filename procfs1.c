#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


#define procfs_name "zero"
#define PROCFS_MAX_SIZE 1024

struct proc_dir_entry *Our_Proc_File;
static char procfs_buffer[PROCFS_MAX_SIZE];

static unsigned long procfs_buffer_size = 0;

int procfile_read(  char *buffer,
		            char **buffer_location,
		            off_t offset,
                    int buffer_length,
                    int *eof,
                    void *data)
{

	int ret;
	printk(KERN_INFO "procfile_read(/proc/%s) called \n", procfs_name);

	if ( offset > 0) {
		ret = 0;
	} else {

        memcpy(buffer, procfs_buffer, procfs_buffer_size);
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
    if ( procfs_buffer_size > PROCFS_MAX_SIZE) {
        procfs_buffer_size = PROCFS_MAX_SIZE;
    }

    if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
        return -EFAULT;
    }
    return procfs_buffer_size;
}


int init_module()
{
	Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL);

	if ( Our_Proc_File == NULL) {
		remove_proc_entry(procfs_name, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s \n", procfs_name);
		return -ENOMEM;
	}

	Our_Proc_File->read_proc	= procfile_read;
	Our_Proc_File->write_proc   = procfs_write;
    //Our_Proc_File->owner		= THIS_MODULE;
	Our_Proc_File->mode		= S_IFREG | S_IRUGO;
	Our_Proc_File->uid		= 0;
	Our_Proc_File->gid		= 0;
	Our_Proc_File->size		= 37;

	printk(KERN_INFO "/proc/%s created \n", procfs_name);
	return 0;
}

void cleanup_module()
{

	remove_proc_entry(procfs_name, NULL);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}
