#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/platform_data/dma-imx.h>

#include <linux/dmaengine.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/delay.h>

#define EIM_ADDR        0x08000000

static int gMajor; /* major number of device */
static struct class *dma_tm_class;
u32 *wbuf;

int cnt;

ssize_t eim_read (struct file *filp, char __user * buf, size_t count,
								loff_t * offset)
{
	int i;

	printk("addr[0x%08x]   -->  data[0x%08x]\n", EIM_ADDR + i, *(wbuf + i));
	++i;
	
	return 0;
}

struct file_operations dma_fops = {
	read:		eim_read,
};

int __init eim_init_module(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct device *temp_class;
#else
	struct class_device *temp_class;
#endif
	int error;

/////////////////////////////////////////////////////////////////
	wbuf = (u32 *)ioremap(DATA_SOURCE_ADDR, BUFF_DATA_LENGTH);
	printk("ioremap wbuf succeed\n");
/////////////////////////////////////////////////////////////////

	/* register a character device */
	error = register_chrdev(0, "eim_test", &dma_fops);
	if (error < 0) {
		printk("EIM test driver can't get major number\n");
		return error;
	}
	gMajor = error;
	printk("EIM test major number = %d\n",gMajor);

	dma_tm_class = class_create(THIS_MODULE, "ein_test");
	if (IS_ERR(dma_tm_class)) {
		printk(KERN_ERR "Error creating eim test module class.\n");
		unregister_chrdev(gMajor, "eim_test");
		return PTR_ERR(dma_tm_class);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
	temp_class = device_create(dma_tm_class, NULL,
				   MKDEV(gMajor, 0), NULL, "eim_test");
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	temp_class = device_create(dma_tm_class, NULL,
				   MKDEV(gMajor, 0), "eim_test");
#else
	temp_class = class_device_create(dma_tm_class, NULL,
					     MKDEV(gMajor, 0), NULL,
					     "eim_test");
#endif
	if (IS_ERR(temp_class)) {
		printk(KERN_ERR "Error creating eim test class device.\n");
		class_destroy(dma_tm_class);
		unregister_chrdev(gMajor, "eim_test");
		return -1;
	}

	printk("EIM test Driver Module loaded\n");
	return 0;
}

static void eim_cleanup_module(void)
{
////////////////////////////////////////////
	iounmap(wbuf);
////////////////////////////////////////////

	unregister_chrdev(gMajor, "eim_test");
	device_destroy(dma_tm_class, MKDEV(gMajor, 0));
	class_destroy(dma_tm_class);

	printk("EIM test Driver Module Unloaded\n");
}


module_init(eim_init_module);
module_exit(eim_cleanup_module);

MODULE_DESCRIPTION("EIM test driver");
MODULE_LICENSE("GPL");
