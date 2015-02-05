#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

int len,temp;
static int board[9] = {-1};
static int turn = 0;
// static 

struct player {
	char name[10];
	struct proc_dir_entry *root;
	struct proc_dir_entry *game;
	char *buffer_game;
	char *buffer_opponent;
	// char *buffer_game;
	struct proc_dir_entry *opponent;
};

static struct player m_ninja;
static struct player m_saber;
struct playerList {
	int numPlyaer;
	struct player *ninja;
	struct player *saber;
};

static struct playerList *m_list;


void registerPlyaer(char* uname, struct player *mm_list, int mode);
void remove_user(struct player *mm_list);


// I don't figure out the API directly point to the file buffer. 
// so i have to create four buffers for two user. different write and read functions hard-coded buffer address for them. 

int read_game_ninja(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
		if(count>temp)
		{
			count=temp;
		}
		temp=temp-count;
		copy_to_user(buf, (*(m_list->ninja)).buffer_game, count);
		if(count==0)
			temp=len;
   
	return count;
}
int read_opponent_ninja(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
		if(count>temp)
		{
			count=temp;
		}
		temp=temp-count;
		copy_to_user(buf, (*(m_list->ninja)).buffer_opponent, count);
		if(count==0)
			temp=len;
   
	return count;
}

int write_game_ninja(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	copy_from_user((*(m_list->ninja)).buffer_game,buf,count);
	len=count;
	temp=len;
	return count;
}
int write_opponent_ninja(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	char getName[10];
	// only there is a valid user.
	if ( m_list->numPlyaer == 1 )
	{
		 copy_from_user(getName,buf,count);
		 memcpy((*(m_list->ninja)).buffer_opponent,getName,count);
		 len=count;
		 temp=len;
		 // copy_from_user(getName,buf,count);
 		printk(GFP_KERNEL, "Register: user %s is registing, will remove when game end", getName);

		 registerPlyaer(getName, m_list->saber, 0);
 		// start the game

	}

	return count;
}


int read_game_saber(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
		if(count>temp)
		{
			count=temp;
		}
		temp=temp-count;
		copy_to_user(buf, (*(m_list->saber)).buffer_game, count);
		if(count==0)
			temp=len;
   
	return count;
}
int read_opponent_saber(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
		if(count>temp)
		{
			count=temp;
		}
		temp=temp-count;
		copy_to_user(buf, (*(m_list->saber)).buffer_opponent, count);
		if(count==0)
			temp=len;
   
	return count;
}

int write_game_saber(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	copy_from_user((*(m_list->saber)).buffer_game,buf,count);
	len=count;
	temp=len;
	return count;
}
int write_opponent_saber(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	char getName[10];
	if ( m_list->numPlyaer == 1 )
	{
		copy_from_user(getName,buf,count);
		 memcpy((*(m_list->saber)).buffer_opponent,getName,count);
		len=count;
		temp=len;
		printk(GFP_KERNEL, "Register: user %s is registing, will remove when game end", getName);
		registerPlyaer(getName, m_list->ninja, 1);
		// start the game
	}
	return count;
}

struct file_operations game_fops_ninja = {
	read: read_game_ninja,
	write: write_game_ninja
};

struct file_operations opp_fops_ninja = {
	read: read_opponent_ninja,
	write: write_opponent_ninja
};

struct file_operations game_fops_saber = {
	read: read_game_saber,
	write: write_game_saber
};

struct file_operations opp_fops_saber = {
	read: read_opponent_saber,
	write: write_opponent_saber
};

// mode means we need to know which ninja or saber you are reginered. 
// 1 = ninja  or 0 = saber
void registerPlyaer(char* uname, struct player *mm_list, int mode) 
{
	m_list->numPlyaer++;
	strcpy(mm_list->name, uname);
	struct proc_dir_entry *m_dir_parent;
	struct proc_dir_entry *m_dir_entry;
	m_dir_parent = proc_mkdir(uname, NULL);
    if ( m_dir_parent == NULL) {
        remove_proc_entry(uname, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s \n", uname);
        // return -ENOMEM;
    }
    mm_list->root = m_dir_parent;
    if ( mode )
	    m_dir_entry = proc_create("game", 0666, m_dir_parent, &game_fops_ninja);
    else
	    m_dir_entry = proc_create("game", 0666, m_dir_parent, &game_fops_saber);

    if ( m_dir_entry == NULL) {
        remove_proc_entry(uname, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s/game \n", uname);
        // return -ENOMEM;
    }else {
    	mm_list->game = m_dir_entry;
    	mm_list->buffer_game = kmalloc(GFP_KERNEL, 10*sizeof(char));
    }

    if ( mode )
	    m_dir_entry = proc_create("opponent", 0666, m_dir_parent, &opp_fops_ninja);
    else 
	    m_dir_entry = proc_create("opponent", 0666, m_dir_parent, &opp_fops_saber);

    if ( m_dir_entry == NULL) {
        remove_proc_entry(uname, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s/opponent \n", uname);
        // return -ENOMEM;
    }else {
    	mm_list->opponent = m_dir_entry;
    	mm_list->buffer_opponent = kmalloc(GFP_KERNEL, 10*sizeof(char));
    }
    printk(KERN_ALERT "Register: User %s is registed into List \n", uname);


	// proc_create("hello",0,NULL,&proc_fops);
	// msg=kmalloc(GFP_KERNEL,10*sizeof(char));
}

void remove_user(struct player *mm_list) 
{
    printk(KERN_ALERT "Register: User %s is removed from List \n", mm_list->name);
	m_list->numPlyaer--;
	kfree(mm_list->buffer_opponent);
	kfree(mm_list->buffer_game);
	remove_proc_entry("game",mm_list->root);
	remove_proc_entry("opponent", mm_list->root);
	remove_proc_entry(mm_list->name, NULL);
	memset(mm_list,0,sizeof(struct player));

}

int proc_init (void) {

 m_list = kmalloc(GFP_KERNEL,sizeof(struct playerList));
 m_list->numPlyaer = 0;
 m_list->ninja = &m_ninja;
 m_list->saber = &m_saber;


 registerPlyaer("xxxx", m_list->ninja, 1);
 // registerPlyaer("miao", m_list->saber, 0);

 return 0;
}

void proc_cleanup(void) {
	remove_user(m_list->ninja); 
	// remove_user(m_list->saber);
	kfree(m_list);

}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);
