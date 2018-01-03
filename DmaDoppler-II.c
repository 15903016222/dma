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
#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/kthread.h>
#include <linux/dmaengine.h>
#include <linux/platform_data/dma-imx.h>
#include <linux/platform_data/dma-imx-sdma.h>

#include <linux/workqueue.h> //

#include <net/netlink.h>
#include <net/sock.h>

#define NETLINK_TEST (25)
#define NLMSG_PID    (100)

#define IMX_GPIO_NR(bank, nr)  (((bank) - 1) * 32 + (nr))
#define GPIO_IRQ(bank, nr)     (gpio_to_irq (IMX_GPIO_NR((bank), (nr))))

#define VERSION "20170327"

#define	FB_FRAME_SIZE	  768
#define DATA_FRAME_SIZE  1024
#define BUFF_FRAME_SIZE   512
#define FB_FRAME_COUNT    400
#define DATA_FRAME_COUNT   96
#define BUFF_FRAME_COUNT 1536

#define FB_DMA_SIZE     768 * 400 * 2

#define DMA_SOURCE_ADDR          0x08000000

#define DMA_START_ADDR           0x2f000000     // 496 -- 512M
#define DMA_DATA_LENGTH          0x01000000     // 16M

#define BUFF_START_ADDR          0x30000000
#define BUFF_DATA_LENGTH         0x20000000

#define CONFIG_START_ADDR_OFFSET 0x00100000
#define SCAN_DATA_MARK_OFFSET    0x00200000
#define CONFIG_DATA_LENGTH       0x00000100

#define REGION_SIZE              0x00040000     // 256K
#define DATA_SAVE_BLOCK_SIZE_BIT     1024
#define DATA_SAVE_BLOCK_SIZE_WORD     512

#define DmaFrameBuffer         config[0]
#define DataDmaCounter         config[1]
#define BufferInUse            config[2]
#define ScanSource             config[3]
#define StoreFrameCount        config[4]
#define EncoderCounterOffset   config[5]
#define StepsPerResolution     config[6]
#define ScanZeroIndexOffset    config[7]
#define MaxStoreIndex          config[8]
#define ScanTimmerCounter      config[9]
#define ScanTimmerCircled      config[10] 

#define CONFIG_NUM 11

struct sock *nl_sk = NULL;

static struct file *fp = NULL;
static mm_segment_t fs;
#define BUFFER_SIZE 4096

int config_atomic[CONFIG_NUM];
#define DmaFrameBufferAtomic         config_atomic[0]
#define DataDmaCounterAtomic         config_atomic[1]
#define BufferInUseAtomic            config_atomic[2]
#define ScanSourceAtomic             config_atomic[3]
#define StoreFrameCountAtomic        config_atomic[4]
#define EncoderCounterOffsetAtomic   config_atomic[5]
#define StepsPerResolutionAtomic     config_atomic[6]
#define ScanZeroIndexOffsetAtomic    config_atomic[7]
#define MaxStoreIndexAtomic          config_atomic[8]
#define ScanTimmerCounterAtomic      config_atomic[9]
#define ScanTimmerCircledAtomic      config_atomic[10]


volatile unsigned char* scan_mark  ;
volatile int* config;            // config[0]  DRAW condition
// config[1]  DMA counter
// config[2]  which Buffer of the four is in use by ARM
// config[3]  scan source 0: timmer 1: encoder
// config[4]  DMA DATA SIZE  (512B per unit)
// config[5]  PointQty + DMA_OFFSET + 4(5) * 4  (encoder data position)
// config[6]  StepPerResolution
// config[7]  Scan Zero Position Offset
// config[8]  Max store index
// config[9]  Scan Timer counter
// config[10] whether the data counter is circled in 256M memory

int OFFSET   ;
int OFFSET_ADDR[4] = {
    DMA_START_ADDR ,
    DMA_START_ADDR +     REGION_SIZE ,
    DMA_START_ADDR + 2 * REGION_SIZE ,
    DMA_START_ADDR + 3 * REGION_SIZE
} ;

int OFFSET_VIRTUAL_ADDR[4];

struct thread_data {
    int nr;
    pid_t pid;
    char * name;
};

struct dma_transfer {
    struct dma_chan *ch;
    struct dma_slave_config dma_m2m_config;
    struct dma_async_tx_descriptor *dma_m2m_desc;
    struct completion dma_m2m_ok;
    unsigned int   phys_from;
    unsigned int   phys_to;
    unsigned short data_type;	/* Data length */
    unsigned short frame_size;	/* Elements to transfer */
    unsigned short frame_count;	/* Framses to transfer */
    unsigned int   status;		/* Callback status */
};

static int  dmatest_work(void *data);
static void dma_memcpy_callback_from_fpga(void *data);

static void netlink_send(void);

static unsigned int  data_addr ;
static struct dma_transfer dma_data   ; 
static bool dma_m2m_filter (struct dma_chan *chan, void *param)
{
    if (!imx_dma_is_general_purpose(chan))
        return false;
    chan->private = param;
    return true;
}

void save_dma_data_file (void)
{
    int  EncoderIndex   ;
    int* pEncoderIndex  ;
    int  nOffset        ;
    loff_t pos;

    if(ScanSourceAtomic) {
        /* encoder */
        nOffset  = DataDmaCounterAtomic & 0x00000003 ;
        pEncoderIndex = & EncoderIndex ;
        memcpy((void *)pEncoderIndex,
               (void *)(nOffset *  REGION_SIZE + EncoderCounterOffsetAtomic + data_addr),
               4);
        EncoderIndex  = EncoderIndex / StepsPerResolutionAtomic + ScanZeroIndexOffsetAtomic;
        if(EncoderIndex > MaxStoreIndexAtomic || EncoderIndex < 0) {
            /* out of range , do not dma */
            return ;
        }
        scan_mark[EncoderIndex] = 0xff ;
        pos  = EncoderIndex * StoreFrameCountAtomic * BUFFER_SIZE  ;
    }
    else {
        /* time */
        if(ScanTimmerCounterAtomic > MaxStoreIndexAtomic) {
            /* out of range , restart from 0x90000000 */
            ScanTimmerCounter = 0 ;
            ScanTimmerCircled++ ;
        }
        scan_mark[ScanTimmerCounterAtomic]  = 0xff ;
        pos = ScanTimmerCounterAtomic * BUFFER_SIZE;    // address to store
    }

    if(dma_data.frame_count != StoreFrameCount) {
        /* frame count */
        dma_data.frame_count = StoreFrameCount  ;
    }

    set_fs(KERNEL_DS);
    vfs_write(fp,
              (const char *)OFFSET_VIRTUAL_ADDR[DataDmaCounterAtomic & 0x00000003],
              (StoreFrameCountAtomic / 4 + 1) * BUFFER_SIZE,
              &pos);

    ScanTimmerCounter++;
}


/*
 * The callback gets called by the DMA interrupt handler after
 * the transfer is complete.
 */
static void dma_memcpy_callback_from_fpga(void *data)
{
    DmaFrameBuffer = 0xfffffff ;
    memcpy ((void *)config_atomic, (void *)config, CONFIG_NUM * sizeof (int));
    netlink_send();

    do {
        config[1]++ ;
        OFFSET = config[1] & 0x00000003 ;
    } while(OFFSET == config[2]) ;
    
    if (dma_data.frame_count != StoreFrameCount) {
        dma_data.frame_count = StoreFrameCount ;
    }
    dma_data.phys_to = OFFSET_ADDR[OFFSET];

    dma_data.dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma_data.dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma_data.ch, &dma_data.dma_m2m_config);
    dma_data.dma_m2m_desc = dma_data.ch->device->device_prep_dma_memcpy(dma_data.ch,
                                        dma_data.phys_to, dma_data.phys_from,
                                        dma_data.data_type * dma_data.frame_size * dma_data.frame_count,0);
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
    dma->phys_from	= DMA_SOURCE_ADDR;
    dma->phys_to	= DMA_START_ADDR ;
    dma->data_type = 0x02;
    dma->status = 0;
    dma->frame_size  =  DATA_SAVE_BLOCK_SIZE_WORD ; //DATA_FRAME_SIZE ;
    dma->frame_count =  192; //DATA_FRAME_COUNT;

    /* Get a DMA channel */
    dma->ch = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
    if (!dma->ch) {
        printk(KERN_ERR "Could not get DMA with dma_request_channel()\n");
        return -ENOMEM;
    }

    dma->dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma->dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma->ch, &dma->dma_m2m_config);
    dma->dma_m2m_desc = dma->ch->device->device_prep_dma_memcpy(dma->ch,
                                        dma->phys_to, dma->phys_from,
                                        dma->data_type * dma->frame_size * dma->frame_count,0);
    dma->dma_m2m_desc->callback = dma_memcpy_callback_from_fpga;
    dmaengine_submit(dma->dma_m2m_desc);

    return 0;
}

static irqreturn_t dma_start (int irq, void *dev_id)
{
    struct dma_transfer* dma = &dma_data ;

    dma_async_issue_pending (dma->ch);

	return IRQ_HANDLED;
}

static int dmatest_work (void *data)
{
    int ret;

    allow_signal(SIGTERM);
    current->state = TASK_INTERRUPTIBLE;
    do{
        schedule_timeout(HZ * 1);
    }while(!StoreFrameCount) ;
    current->state = TASK_RUNNING;
    /* FIXME: Does not free mem in SIGTERM currently */
    if (signal_pending(current)) {
        flush_signals(current);
    }

    /* Set up transfer data */
    // DMA frome gpmc/eim to Main memory
    dma_mem_transfer_from_fpga();

	gpio_request (IMX_GPIO_NR (7, 11), "GPIO_16");
	gpio_direction_input (IMX_GPIO_NR (7, 11));

	ret = request_irq (GPIO_IRQ (7, 11), dma_start,
						IRQF_TRIGGER_FALLING, "dma_irq", NULL);
	if (ret) {
		printk ("request_irq failed \n");
		return ret;
	}

    return 0;
}

static void netlink_send(void)
{
    struct sk_buff *skb_send;
    struct nlmsghdr *nlh_send;

    if (!nl_sk) {
        return;
    }
    skb_send = alloc_skb(NLMSG_SPACE(0), GFP_ATOMIC);
    if (!skb_send) {
        printk(KERN_ERR "alloc_skb error!\n");
        return ;
    }
    nlh_send = nlmsg_put(skb_send, 0, 0, 0, 0, 0);
    NETLINK_CB(skb_send).portid = 0;
    NETLINK_CB(skb_send).dst_group = 0;
    netlink_unicast(nl_sk, skb_send, NLMSG_PID, MSG_DONTWAIT);

    return ;
}

static int bDmaStoreProcessing = 0;
static void netlink_input(struct sk_buff *__skb)
{
    struct sk_buff *skb;

    skb = skb_get(__skb);
    if( skb->len < NLMSG_SPACE(0)) {
            return;
    }
    if (bDmaStoreProcessing) {
        kfree_skb (__skb);
        return ;
    }
    bDmaStoreProcessing = 1;
    save_dma_data_file ();
    kfree_skb (__skb);
    bDmaStoreProcessing = 0;
    return;
}

static char *name = "dmatest";

static int __init dmatest_init(void)
{
    struct thread_data * thread;

    struct netlink_kernel_cfg nkc;

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
    data_addr = (unsigned int )ioremap(DMA_START_ADDR, DMA_DATA_LENGTH);
    memset((void*)data_addr , 0 , 0x100100) ;
    OFFSET_VIRTUAL_ADDR[0] = data_addr;
    OFFSET_VIRTUAL_ADDR[1] = data_addr + REGION_SIZE;
    OFFSET_VIRTUAL_ADDR[2] = data_addr + REGION_SIZE * 2;
    OFFSET_VIRTUAL_ADDR[3] = data_addr + REGION_SIZE * 3;

    config = (unsigned int*)(data_addr + CONFIG_START_ADDR_OFFSET);
    scan_mark = (char*)(data_addr + SCAN_DATA_MARK_OFFSET)  ;

    printk("<0>config addr %x  data_addr %x \n", (int)config , (int)data_addr);
    printk("<0>dma module is running !\n");

    // init netlink
    nkc.groups = 0;
    nkc.flags = 0;
    nkc.input = netlink_input;
    nkc.cb_mutex = NULL;
    nkc.bind = NULL;
    nkc.unbind = NULL;
    nkc.compare = NULL;
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &nkc);
    if (!nl_sk) {
        printk(KERN_ERR "[netlink] create netlink socket error!\n");
        goto err;
    }

    fp = filp_open("/media/sata/kernel_file", O_RDWR | O_CREAT, 0644);
    if (IS_ERR(fp)) {
        printk("create file error\n");
        return -1;
    }
    fs = get_fs();

    printk ("sizeof(loff_t) = %d \n", sizeof (loff_t));
    printk("<0>netlink init success!\n");
    return 0;

err:
    netlink_kernel_release(nl_sk);
free_threads:
    kfree(thread);

    return -ENOMEM;
}

static void __exit dmatest_exit(void)
{
    filp_close(fp, NULL);
    set_fs(fs);

    return;
}

MODULE_AUTHOR("Doppler");
MODULE_DESCRIPTION("Phascan DMA module");
MODULE_LICENSE("Dual BSD/GPL");
module_init(dmatest_init);
module_exit(dmatest_exit);
