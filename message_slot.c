#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>  
MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"

typedef struct channels{
    char message[MESSAGE_LEN];
    int length;
    struct channels *next;
    long id;
}channel;

static channel *slots[SLOTS_NUM];


static int device_open( struct inode* inode,struct file*  file ){
    channel *ch;
    unsigned int minor_num=iminor(inode);
    if(slots[minor_num]==NULL){
        ch=kmalloc(sizeof (channel),GFP_KERNEL);
        if(ch==NULL){
            return -1;
        }
        slots[minor_num]=ch;
    }
	printk("Open is sucessful\n");
    return 0;
}
static ssize_t device_read(struct file* file,char __user* buffer,size_t length,loff_t* offset ){
    int i;
    unsigned int minor=iminor(file->f_path.dentry->d_inode);
    channel *ch=slots[minor]->next;;
printk("read\n");
    if(ch==NULL||buffer==NULL){
        return -EINVAL;
    }
printk("ch not null read/n");
    if(ch->length==0){
        return -EWOULDBLOCK;
    }
    if(length<ch->length){
        return -ENOSPC;
    }
    for( i = 0; i < ch->length ; i++ ){

        if( put_user(ch->message[i], &buffer[i])!=0 ){
            break;
        }

    }
    if(i==ch->length){
printk("read: success\n");
        return i;
    }
    return -1;
}

static ssize_t device_write( struct file* file,const char __user* buffer,size_t length,loff_t* offset){
    int i;
    unsigned int minor=iminor(file->f_path.dentry->d_inode);

    channel *ch=slots[minor]->next;
printk("write\n");
    if(ch==NULL||buffer==NULL){
        return -EINVAL;
    }
printk("ch not null/n");
    if(length<=0 || length > MESSAGE_LEN){
        return -EMSGSIZE;
    }
    for( i = 0; i < length ; i++ ){

        if( get_user(ch->message[i], &buffer[i])!=0 ){
            break;
        }
    }
    ch->length=0;
    if(i==length){
        ch->length=i;
    }
printk("write : success\n");
    return(i);

}
int find_by_id(unsigned int minor,unsigned long  ioctl_param ){

    channel *ch_head=slots[minor];
    channel *ch_new,*ch_prev,*ch_curr;
    ch_prev=ch_head;
    ch_curr=ch_head;
printk("find by id\n");
    while(ch_curr!=NULL){
        if(ch_curr->id==ioctl_param){
printk("find by id: id exists\n");
            ch_prev->next=ch_curr->next;
            ch_curr->next=ch_head->next;
            ch_head->next=ch_curr;
printk("find by id: success\n");
            return 0;
        }
        ch_prev=ch_curr;
        ch_curr=ch_curr->next;
    }
	printk("find by id: alocate new channel\n");
    ch_new=kmalloc(sizeof (channel),GFP_KERNEL);
    if(ch_new==NULL){
        return -1;
    }
    ch_new->id=ioctl_param;
    ch_new->next=ch_head->next;
    ch_new->length=0;
    ch_head->next=ch_new;

	printk("find by id: success\n");
    return 0;

}
static long device_ioctl( struct   file* file,unsigned int   ioctl_command_id,unsigned long  ioctl_param ){
    unsigned int minor=iminor(file->f_path.dentry->d_inode);
    int ch_get;
    if(MSG_SLOT_CHANNEL!=ioctl_command_id || ioctl_param==0){
        return -EINVAL;
    }
    ch_get=find_by_id(minor,ioctl_param);
    if(ch_get<0){
        return -1;
    }
printk("ioctal: success\n");
    return 0;



}
static int device_release( struct inode* inode,struct file*  file){
	return 0;

}
struct file_operations Fops =
        {
                .owner	  = THIS_MODULE,
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .unlocked_ioctl = device_ioctl,
                .release        = device_release,
        };

static int __init simple_init(void){
    int rc = -1;
    rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
    if(rc<0){
        printk( KERN_ALERT "%s registraion failed for  %d\n",DEVICE_RANGE_NAME, MAJOR_NUM );
        return rc;
    }
    printk( "\n");
 printk( "Registeration is successful.\n");
 printk( "\n");
    return 0;
}
static void __exit simple_cleanup(void)
{
    int i;
    channel *ch,*head;
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
    for(i=0; i<SLOTS_NUM;i++){
        head=slots[i];
        while(head!=NULL){
            ch=head;
            head=head->next;
            kfree(ch);
        }
    }
}
module_init(simple_init);
module_exit(simple_cleanup);