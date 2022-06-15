#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define HELLO_MAJOR  1
#define HELLO_MINOR  0
#define HELLO_COUNT  1
#define BUFFER_SIZE  1024

MODULE_AUTHOR("Francesco Benetello <francesco.benetello921@gmail.com>");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int hello_open(struct inode *inode, struct file *file);
static ssize_t hello_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset);
static ssize_t hello_write(struct file *file, const char __user *user_buffer, size_t size, loff_t * offset);
static int hello_release(struct inode * inode, struct file * file);

struct hello_device_data {
	struct cdev cdev;
	/* Hello Device Data */
	char buffer[BUFFER_SIZE];
	int size;
};

struct hello_device_data devs;
static struct class *hello_class = NULL;
static int hello_major = 0;

const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.release = hello_release,
	.unlocked_ioctl = hello_ioctl
};

static int hello_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var (env, "DEVMODE=%#o", 0666);
	return 0;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO "chrdevice.c: Device ioctl\n");
	return 0;
}

/* Invoked when module start, 'insmod command' */
/* Create one device named 'hello' */
int init_module(void)
{
	int err;
	dev_t devno;

	err = alloc_chrdev_region(&devno, 0, 0, "hello");

	if (err != 0) {
		printk(KERN_INFO "chrdevice.c: Device allocation error, error code: %d", err);
		return err;
	}

	hello_major = MAJOR(devno);
	
	hello_class = class_create(THIS_MODULE, "hello");
	hello_class->dev_uevent = hello_uevent;

	devs.size = BUFFER_SIZE;
	cdev_init(&devs.cdev, &hello_fops);
	devs.cdev.owner = THIS_MODULE;
	err = cdev_add(&devs.cdev, MKDEV(hello_major, 0), HELLO_COUNT);

	if (err != 0) {
		printk(KERN_INFO "chrdevice.c: Device init error, error code: %d", err);
		return -1;
	}

	device_create(hello_class, NULL, MKDEV(hello_major, 0), NULL, "hello", 0);

	printk(KERN_INFO "chrdevice.c: Device init completed successfully");

	return 0;
}

/* Invoke when Module end, 'rmmod command' */
void cleanup_module(void)
{
	device_destroy (hello_class, MKDEV (hello_major, 0));
	class_unregister (hello_class);
	class_destroy (hello_class);

	unregister_chrdev_region(MKDEV(hello_major, 0), MINORMASK);
	printk(KERN_INFO "chrdevice.c: Device cleanup completed");
}

static int hello_open(struct inode *inode, struct file *file)
{
	struct hello_device_data *data;

	data = container_of(inode->i_cdev, struct hello_device_data, cdev);

	file->private_data = data;

	printk(KERN_INFO "chrdevice.c: Device open successfully");
	return 0;
}

static ssize_t hello_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset)
{
	struct hello_device_data *hello_data = (struct hello_device_data *) file->private_data;

	int bytes = size;
	if (*offset >= hello_data->size) {
		return 0;
	}

	if (bytes > (hello_data->size - *offset)) {
		bytes = hello_data->size - *offset;
	}


	if (copy_to_user(user_buffer, hello_data->buffer + *offset, bytes)) {
		return -EFAULT;
	}

	printk(KERN_INFO "chrdevice.c: Device read %d bytes", bytes);

	*offset += bytes;
	return bytes;
}

static ssize_t hello_write(struct file *file, const char __user *user_buffer, size_t size, loff_t * offset)
{
	struct hello_device_data *hello_data = (struct hello_device_data *) file->private_data;

	int bytes = size;
	if (*offset >= hello_data->size) {
		return 0;
	}

	if (bytes > (hello_data->size - *offset)) {
		bytes = hello_data->size - *offset;
	}


	if (copy_from_user(hello_data->buffer + *offset, user_buffer, bytes)) {
		return -EFAULT;
	}

	printk(KERN_INFO "chrdevice.c: Device write %d bytes", bytes);

	*offset += bytes;
	return bytes;
}

static int hello_release(struct inode * inode, struct file * file)
{
	printk(KERN_INFO "chrdevice.c: Device released");
	return 0;
}