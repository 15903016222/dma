#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/delay.h>

#include <linux/dmaengine.h>
#include <linux/device.h>
#include <linux/platform_data/dma-imx.h>
#include <linux/platform_data/dma-imx-sdma.h>

#include <linux/io.h>
#include <linux/delay.h>

static int gMajor; /* major number of device */
static struct class *dma_tm_class;
u32 *wbuf, *wbuf1, *wbuf2, *wbuf3, *wbuf4;
u32 *rbuf, *rbuf1, *rbuf2, *rbuf3, *rbuf4;

struct dma_chan *dma_m2m_chan;

struct completion dma_m2m_ok;

struct scatterlist sg[4], sg2[4];

#define SDMA_BUF_SIZE  1024

static bool dma_m2m_filter(struct dma_chan *chan, void *param)
{
	if (!imx_dma_is_general_purpose(chan))
		return false;
	chan->private = param;
	return true;
}

int sdma_open(struct inode *inode, struct file *filp)
{
	dma_cap_mask_t dma_m2m_mask;
	struct imx_dma_data m2m_dma_data;

	init_completion(&dma_m2m_ok);

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_SLAVE, dma_m2m_mask);
	m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
	m2m_dma_data.priority = DMA_PRIO_HIGH;
	dma_m2m_chan = dma_request_channel(dma_m2m_mask, dma_m2m_filter,
								&m2m_dma_data);
	if (!dma_m2m_chan) {
		printk("Error opening the SDMA memory to memory channel\n");
		return -EINVAL;
	}
	printk ("%s %d \n", __func__, __LINE__);

	wbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf) {
		printk("error wbuf !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	wbuf1 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf1) {
		printk("error wbuf1 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	wbuf2 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf2) {
		printk("error wbuf2 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	wbuf3 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf3) {
		printk("error wbuf3 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	wbuf4 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!wbuf4) {
		printk("error wbuf4 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	rbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf) {
		printk("error rbuf !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	rbuf1 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf1) {
		printk("error rbuf1 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	rbuf2 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf2) {
		printk("error rbuf2 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	rbuf3 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf3) {
		printk("error rbuf3 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	rbuf4 = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if(!rbuf4) {
		printk("error rbuf4 !!!!!!!!!!!\n");
		return -1;
	}
	printk ("%s %d \n", __func__, __LINE__);

	return 0;
}

int sdma_release(struct inode * inode, struct file * filp)
{
	dmaengine_terminate_all(dma_m2m_chan);
	dma_release_channel(dma_m2m_chan);
	dma_m2m_chan = NULL;
	kfree(wbuf);
	kfree(rbuf);
	return 0;
}

ssize_t sdma_read (struct file *filp, char __user * buf, size_t count,
loff_t * offset)
{
	int i;
/*	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf+i) != *(wbuf+i)) {
			printk("buffer 0 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 0 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf1+i) != *(wbuf1+i)) {
			printk("buffer 1 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 1 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf2+i) != *(wbuf2+i)) {
			printk("buffer 2 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 2 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf3+i) != *(wbuf3+i)) {
			printk("buffer 3 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 3 copy passed!\n");
*/
	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf4+i) != *(wbuf4+i)) {
			printk("buffer 4 copy falled!\n");
			return 0;
		}
	}
	printk("buffer 4 copy passed!\n");

	return 0;
}


static void dma_m2m_callback(void *data)
{
	printk ("%s %s %d \n", __FILE__, __func__, __LINE__);
	complete(&dma_m2m_ok);
	return ;
}

ssize_t sdma_write(struct file * filp, const char __user * buf, size_t count,
        loff_t * offset)
{
	 u32 *index1, *index2, *index3, i, ret;
	 struct dma_slave_config dma_m2m_config = {0};
	 struct dma_async_tx_descriptor *dma_m2m_desc;
	 u32 *index4 = wbuf4;
	 dma_addr_t dma_src, dma_dst;

	 index1 = wbuf;
	 index2 = wbuf2;
	 index3 = wbuf3;

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		  *(index1 + i) = 0x12121212;
	 }
	printk ("%s %d \n", __func__, __LINE__);

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		  *(index2 + i) = 0x34343434;
	 }
	printk ("%s %d \n", __func__, __LINE__);

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		  *(index3 + i) = 0x56565656;
	 }
	printk ("%s %d \n", __func__, __LINE__);

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		  *(index4 + i) = 0x78787878;
	 }
	printk ("%s %d \n", __func__, __LINE__);

#if 0
	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		 printk("input data_%d : %x\n", i, *(wbuf+i));
	 }

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		 printk("input data2_%d : %x\n", i, *(wbuf2+i));
	 }

	 for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		 printk("input data3_%d : %x\n", i, *(wbuf3+i));
	 }
#endif
	 dma_m2m_config.direction = DMA_MEM_TO_MEM;
	 dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	 dmaengine_slave_config(dma_m2m_chan, &dma_m2m_config);

/*	 sg_init_table(sg, 3);
	 sg_set_buf(&sg[0], wbuf, SDMA_BUF_SIZE);
	 sg_set_buf(&sg[1], wbuf2, SDMA_BUF_SIZE/2);
	 sg_set_buf(&sg[2], wbuf3, SDMA_BUF_SIZE);
	 ret = dma_map_sg(NULL, sg, 3, dma_m2m_config.direction);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,0,35))
	 dma_m2m_desc = dma_m2m_chan->device->device_prep_slave_sg(dma_m2m_chan,sg, 3, dma_m2m_config.direction, 1);
#endif

	 sg_init_table(sg2, 3);
	 sg_set_buf(&sg2[0], rbuf, SDMA_BUF_SIZE);
	 sg_set_buf(&sg2[1], rbuf2, SDMA_BUF_SIZE/2);
	 sg_set_buf(&sg2[2], rbuf3, SDMA_BUF_SIZE);
	 ret = dma_map_sg(NULL, sg2, 3, dma_m2m_config.direction);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,0,35))
	printk ("dma_m2m_chan->device->device_prep_slave_sg = 0x%x\n", (u32)dma_m2m_chan->device->device_prep_slave_sg);
	 dma_m2m_desc = dma_m2m_chan->device->device_prep_slave_sg(dma_m2m_chan,sg2, 3, dma_m2m_config.direction, 0);
	printk ("dma_m2m_desc = 0x%x \n", (u32)dma_m2m_desc);
#else
	printk (" dma_m2m_chan->device->device_prep_dma_sg = 0x%x\n", (u32)dma_m2m_chan->device->device_prep_dma_sg);
	dma_m2m_desc = dma_m2m_chan->device->device_prep_dma_sg(dma_m2m_chan,sg2, 3, sg, 3, 0);
	printk ("dma_m2m_desc = 0x%x \n", (u32)dma_m2m_desc);
#endif

	 dma_m2m_desc->callback = dma_m2m_callback;
	 dmaengine_submit(dma_m2m_desc);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,35))
	 dma_async_issue_pending(dma_m2m_chan);
#endif

	 wait_for_completion(&dma_m2m_ok);
	 dma_unmap_sg(NULL, sg, 3, dma_m2m_config.direction);
	 dma_unmap_sg(NULL, sg2, 3, dma_m2m_config.direction);
*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,35))
	 dma_src = dma_map_single(NULL, wbuf4, SDMA_BUF_SIZE, DMA_TO_DEVICE);
	 dma_dst = dma_map_single(NULL, rbuf4, SDMA_BUF_SIZE, DMA_FROM_DEVICE);
	printk ("dma_m2m_chan->device->device_prep_dma_memcpy = 0x%x\n", (u32)dma_m2m_chan->device->device_prep_dma_memcpy);
	 dma_m2m_desc = dma_m2m_chan->device->device_prep_dma_memcpy(dma_m2m_chan, dma_dst, dma_src, SDMA_BUF_SIZE,0);
	printk ("dma_m2m_desc = 0x%x \n", (u32)dma_m2m_desc);
	 if (!dma_m2m_desc) {
		printk("prep error!!\n");
		return -1;
 	 }
	 dma_m2m_desc->callback = dma_m2m_callback;
	 dmaengine_submit(dma_m2m_desc);
	 dma_async_issue_pending(dma_m2m_chan);
	 wait_for_completion(&dma_m2m_ok);
	 dma_unmap_single(NULL, dma_src, SDMA_BUF_SIZE, DMA_TO_DEVICE);
	 dma_unmap_single(NULL, dma_dst, SDMA_BUF_SIZE, DMA_FROM_DEVICE);
#endif

	 return 0;
}


struct file_operations dma_fops = {
	open: sdma_open,
	release: sdma_release,
	read: sdma_read,
	write: sdma_write,
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	device_destroy(dma_tm_class, MKDEV(gMajor, 0));
#else
	class_device_destroy(dma_tm_class, MKDEV(gMajor, 0));
#endif
	class_destroy(dma_tm_class);

	printk("SDMA test Driver Module Unloaded\n");
}

module_init(sdma_init_module);
module_exit(sdma_cleanup_module);
MODULE_LICENSE("GPL");
