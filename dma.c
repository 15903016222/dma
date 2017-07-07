//#include <linux/config.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/mman.h>
#include <linux/fs.h>

#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/dma-mapping.h>

#include <linux/io.h>

#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/kthread.h>
#include <linux/dmaengine.h>
#include <linux/platform_data/dma-imx.h>
#include <linux/platform_data/dma-imx-sdma.h>

#include <linux/workqueue.h> 

#define IMX_GPIO_NR(bank, nr)  (((bank) - 1) * 32 + (nr))
#define GPIO_IRQ(bank, nr)     (gpio_to_irq (IMX_GPIO_NR((bank), (nr))))

#define DMA_SOURCE_ADDR          0x08000000

#define DMA_START_ADDR           0x2f000000 
#define DMA_DATA_LENGTH          0x00020000 

#define BUFF_START_ADDR          0x30000000
#define BUFF_DATA_LENGTH         0x00020000

struct thread_data {
    int nr;
    pid_t pid;
    char * name;
};

int cnt;
int flag = 0;

struct dma_transfer {
    struct dma_chan *ch;
    struct dma_slave_config dma_m2m_config;
    struct dma_async_tx_descriptor *dma_m2m_desc;
    struct completion dma_m2m_ok;
    unsigned int   phys_from;
    unsigned int   phys_to;
};

static int  dmatest_work(void *data);
static void dma_memcpy_callback_from_fpga(void *data);
static int  dma_mem_transfer_to_store_buffer(void) ;

static unsigned int  data_addr ;
static unsigned int  buff_addr ;
//static struct dma_transfer dma_video  ;
static struct dma_transfer dma_data   ; 
static struct dma_transfer dma_buffer ;

static bool dma_m2m_filter (struct dma_chan *chan, void *param)
{
    if (!imx_dma_is_general_purpose(chan))
        return false;
    chan->private = param;
    return true;
}

static void dma_memcpy_callback_to_buffer(void *data)
{
//	printk ("%s [%d] \n", __func__, __LINE__);
//	printk ("*buff_addr = 0x%X \n", *(unsigned int *)buff_addr);
//	printk ("*buff_addr + 0x%08X = 0x%08X \n", BUFF_DATA_LENGTH / 2 - 4, *(unsigned int *)(buff_addr + BUFF_DATA_LENGTH / 2 - 4));
//	printk ("*buff_addr + 0x%08X = 0x%08X \n", BUFF_DATA_LENGTH - 4, *(unsigned int *)(buff_addr + BUFF_DATA_LENGTH - 4));
//	memset ((void *)data_addr, 0, BUFF_DATA_LENGTH);
//	memset ((void *)buff_addr, 0, BUFF_DATA_LENGTH);
	if (cnt % 100 == 99) 
	{
		flag = 0;
		*(unsigned int *)(data_addr + 0x00020000 * 4 + (cnt / 100) * 4) = cnt;
		printk ("DONE .........................\n");
	}
	++cnt;
}

// dma data to setted buffer address
static void dma_to_store_buffer(void)
{
    struct dma_transfer* dma = &dma_buffer ;

    dma->phys_from   =  DMA_START_ADDR + (cnt % 4) * 0x00020000  ;
    dma->phys_to     =  BUFF_START_ADDR + (cnt % 0x1000) * 0x00020000 ;

    dma->dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma->dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma->ch, &dma->dma_m2m_config);
    dma->dma_m2m_desc = dma->ch->device->device_prep_dma_memcpy(dma->ch,
                                        dma->phys_to, dma->phys_from,
                                        0x00020000, 0);
    dma->dma_m2m_desc->callback = dma_memcpy_callback_to_buffer;
    dmaengine_submit(dma->dma_m2m_desc);
    dma_async_issue_pending (dma->ch);
}

// dma to buffer channel init
static int dma_mem_transfer_to_store_buffer(void)
{
    struct dma_transfer* dma = &dma_buffer ;

    dma_cap_mask_t dma_m2m_mask;
    struct imx_dma_data m2m_dma_data = {0};

    dma_cap_zero (dma_m2m_mask);
    dma_cap_set (DMA_SLAVE, dma_m2m_mask);
    m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
    m2m_dma_data.priority = DMA_PRIO_HIGH;

    memset(dma, 0, sizeof(struct dma_transfer));

    dma->ch = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
    if (!dma->ch) {
        printk(KERN_ERR "Could not get DMA with dma_request_channel()\n");
        return -ENOMEM;
    }
    
    return 0;
}

/*
 * The callback gets called by the DMA interrupt handler after
 * the transfer is complete.
 */
static void dma_memcpy_callback_from_fpga(void *data)
{
    struct dma_transfer* dma = &dma_data ;

    dma_to_store_buffer() ;

	dma->phys_from	= DMA_SOURCE_ADDR;
    dma->phys_to	= DMA_START_ADDR + (cnt % 4) * 0x00020000;

    dma_data.dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma_data.dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma_data.ch, &dma_data.dma_m2m_config);
    dma_data.dma_m2m_desc = dma_data.ch->device->device_prep_dma_memcpy(dma_data.ch,
                                        dma_data.phys_to, dma_data.phys_from,
                                        0x00020000, 0);
    dma_data.dma_m2m_desc->callback = dma_memcpy_callback_from_fpga;
    dmaengine_submit(dma_data.dma_m2m_desc);
	
    return ;
}

static int dma_mem_transfer_from_fpga (void)
{
    struct dma_transfer* dma = &dma_data ;

    dma_cap_mask_t dma_m2m_mask;
    struct imx_dma_data m2m_dma_data = {0};

    dma_cap_zero (dma_m2m_mask);
    dma_cap_set (DMA_SLAVE, dma_m2m_mask);
    m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
    m2m_dma_data.priority = DMA_PRIO_HIGH;

    memset(&dma_data, 0, sizeof(struct dma_transfer));
    
    dma->ch = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
    if (!dma->ch) {
        printk(KERN_ERR "Could not get DMA with dma_request_channel()\n");
        return -ENOMEM;
    }

	dma->phys_from	= DMA_SOURCE_ADDR;
    dma->phys_to	= DMA_START_ADDR + (cnt % 4) * 0x00020000;

    dma_data.dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma_data.dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma_data.ch, &dma_data.dma_m2m_config);
    dma_data.dma_m2m_desc = dma_data.ch->device->device_prep_dma_memcpy(dma_data.ch,
                                        dma_data.phys_to, dma_data.phys_from,
                                        0x00020000, 0);
    dma_data.dma_m2m_desc->callback = dma_memcpy_callback_from_fpga;
    dmaengine_submit(dma_data.dma_m2m_desc);

    return 0;
}

static irqreturn_t dma_start (int irq, void *dev_id)
{
	if (flag == 0) {
		printk ("irq is working ... \n");
		flag = 1;
	}

    dma_async_issue_pending (dma_data.ch);

	return IRQ_HANDLED;
}

static int   dmatest_work (void *data)
{
	int ret;
    dma_mem_transfer_from_fpga();
    // DMA data to store buffer
    dma_mem_transfer_to_store_buffer();

	gpio_request (IMX_GPIO_NR (7, 13), "GPIO_18");
	gpio_direction_input (IMX_GPIO_NR (7, 13));
	ret = request_irq (GPIO_IRQ (7, 13), dma_start, 
						IRQF_TRIGGER_FALLING, "dma_irq", NULL);
	if (ret) 
	{
		printk ("<0>request_irq failed \n");
		return ret;
	}

    return 0;

}

static char *name = "dmatest";

static int __init dmatest_init(void)
{
    struct thread_data * thread;
    printk("<0>DOPPLER DMA MODULE START!\n");
    /* Schedule multiple concurrent dma tests */
    thread = kmalloc(sizeof(struct thread_data), GFP_KERNEL);
    if (!thread) {
        goto free_threads;
    }

    memset(thread, 0, sizeof(struct thread_data));
    thread->nr = 1;
    thread->name = name;

    /* Schedule the test thread */
    kthread_run (dmatest_work, thread, thread->name);

    request_mem_region(DMA_START_ADDR, DMA_DATA_LENGTH, "dma_data");
    data_addr = (unsigned int )ioremap(DMA_START_ADDR, DMA_DATA_LENGTH * 5);
	memset ((void *)data_addr, 0, DMA_DATA_LENGTH * 5);

    request_mem_region(BUFF_START_ADDR, BUFF_DATA_LENGTH , "buffer_data");
    buff_addr = (unsigned int )ioremap( BUFF_START_ADDR , BUFF_DATA_LENGTH * 0x1000);
	memset ((void *)buff_addr, 0, DMA_DATA_LENGTH * 0x1000);

    printk("<0>dma module is running !\n");
    return 0;

free_threads:
    kfree(thread);

    return -ENOMEM;
}

static void __exit dmatest_exit(void)
{
    return;
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(dmatest_init);
module_exit(dmatest_exit);
