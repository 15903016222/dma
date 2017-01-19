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

#define DMA_DATA_ADDR           0x30000000
#define DMA_DATA_LENGTH         0x10000000

#define BUFF_DATA_ADDR          0x40000000
#define BUFF_DATA_LENGTH        DMA_DATA_LENGTH

static int gMajor; /* major number of device */
static struct class *dma_tm_class;
u32 *wbuf;
u32 *rbuf;

struct dma_chan *dma_m2m_chan;

struct completion dma_m2m_ok;

static bool dma_m2m_filter(struct dma_chan *chan, void *param)
{
	if (!imx_dma_is_general_purpose(chan))
		return false;
	chan->private = param;
	return true;
}

int sdma_open(struct inode * inode, struct file * filp)
{
	dma_cap_mask_t dma_m2m_mask;
	struct imx_dma_data m2m_dma_data = {0};

	init_completion(&dma_m2m_ok);

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_SLAVE, dma_m2m_mask);
	m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
	m2m_dma_data.priority = DMA_PRIO_HIGH;

	dma_m2m_chan = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
	if (!dma_m2m_chan) {
		printk("Error opening the SDMA memory to memory channel\n");
		return -EINVAL;
	}

	request_mem_region(DMA_DATA_ADDR, DMA_DATA_LENGTH, "wbuf");
	wbuf = (u32 *)ioremap(DMA_DATA_ADDR, DMA_DATA_LENGTH);
	memset((void*)wbuf , 'A' , DMA_DATA_LENGTH) ;
	printk("ioremap wbuf succeed\n");

	request_mem_region(BUFF_DATA_ADDR, BUFF_DATA_LENGTH, "rbuf");
	rbuf = (u32 *)ioremap(BUFF_DATA_ADDR, BUFF_DATA_LENGTH);
	memset((void*)rbuf , 0 , BUFF_DATA_LENGTH) ;
	printk("ioremap rbuf succeed\n");
	
	printk ("wbuf = 0x%x --> rbuf = 0x%x \n", *wbuf, *rbuf);
	printk ("init memory is over . \n");
	
	return 0;
}

int sdma_release(struct inode * inode, struct file * filp)
{
	dma_release_channel(dma_m2m_chan);
	dma_m2m_chan = NULL;
	iounmap(wbuf);
	iounmap(rbuf);
	return 0;
}

ssize_t sdma_read (struct file *filp, char __user * buf, size_t count,
								loff_t * offset)
{
	int i;

	for (i=0; i<DMA_DATA_LENGTH/4; ++i) {
		if (*(rbuf+i) != *(wbuf+i)) {
			printk("buffer  copy failed!,r=%x,w=%x,%d\n", *(rbuf+i), *(wbuf+i), i);
			return 0;
		}
	}
	printk("buffer copy passed!\n");

	return 0;
}

static void dma_m2m_callback(void *data)
{
	printk("in %s\n",__func__);
	complete(&dma_m2m_ok);
	return ;
}

ssize_t sdma_write(struct file * filp, const char __user * buf, size_t count,
								loff_t * offset)
{
	struct dma_slave_config dma_m2m_config = {0};
	struct dma_async_tx_descriptor *dma_m2m_desc;

	dma_m2m_config.direction = DMA_MEM_TO_MEM;
	dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dmaengine_slave_config(dma_m2m_chan, &dma_m2m_config);

	dma_m2m_desc = dma_m2m_chan->device->device_prep_dma_memcpy(dma_m2m_chan, BUFF_DATA_ADDR, DMA_DATA_ADDR, DMA_DATA_LENGTH,0);
	if (!dma_m2m_desc)
		printk("prep error!!\n");
	dma_m2m_desc->callback = dma_m2m_callback;
	dmaengine_submit(dma_m2m_desc);
	dma_async_issue_pending(dma_m2m_chan);

	printk ("transfer start ... \n");
	wait_for_completion(&dma_m2m_ok);
	printk ("transfer finish ... \n");

	return 0;
}

struct file_operations dma_fops = {
	open:		sdma_open,
	release:	sdma_release,
	read:		sdma_read,
	write:		sdma_write,
};

int __init sdma_init_module(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct device *temp_class;
#else
	struct class_device *temp_class;
#endif
	int error;

	/* register a character device */
	error = register_chrdev(0, "sdma_test", &dma_fops);
	if (error < 0) {
		printk("SDMA test driver can't get major number\n");
		return error;
	}
	gMajor = error;
	printk("SDMA test major number = %d\n",gMajor);

	dma_tm_class = class_create(THIS_MODULE, "sdma_test");
	if (IS_ERR(dma_tm_class)) {
		printk(KERN_ERR "Error creating sdma test module class.\n");
		unregister_chrdev(gMajor, "sdma_test");
		return PTR_ERR(dma_tm_class);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
	temp_class = device_create(dma_tm_class, NULL,
				   MKDEV(gMajor, 0), NULL, "sdma_test");
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	temp_class = device_create(dma_tm_class, NULL,
				   MKDEV(gMajor, 0), "sdma_test");
#else
	temp_class = class_device_create(dma_tm_class, NULL,
					     MKDEV(gMajor, 0), NULL,
					     "sdma_test");
#endif
	if (IS_ERR(temp_class)) {
		printk(KERN_ERR "Error creating sdma test class device.\n");
		class_destroy(dma_tm_class);
		unregister_chrdev(gMajor, "sdma_test");
		return -1;
	}

	printk("SDMA test Driver Module loaded\n");
	return 0;
}

static void sdma_cleanup_module(void)
{
	unregister_chrdev(gMajor, "sdma_test");
	device_destroy(dma_tm_class, MKDEV(gMajor, 0));
	class_destroy(dma_tm_class);

	printk("SDMA test Driver Module Unloaded\n");
}


module_init(sdma_init_module);
module_exit(sdma_cleanup_module);

MODULE_DESCRIPTION("SDMA test driver");
MODULE_LICENSE("GPL");
