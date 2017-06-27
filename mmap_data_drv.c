#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h> //mmap

static int dma_data_mmap(struct file *file,
                        struct vm_area_struct *vma)
{
    vma->vm_page_prot = 
        pgprot_noncached(vma->vm_page_prot);
    
    remap_pfn_range(vma, 
                    vma->vm_start,
                    0x2f000000>>12,
                    0x01000000, 
                    vma->vm_page_prot 
                    );
    return 0;
}

static struct file_operations dma_data_fops = {
    .owner = THIS_MODULE,
    .mmap = dma_data_mmap 
};

static struct miscdevice dma_data_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "dma_data",
    .fops = &dma_data_fops
};

static int dma_data_init(void)
{
    misc_register(&dma_data_misc);
	printk ("<0>dma_data module is loaded. \n");
    return 0;
}

static void dma_data_exit(void)
{
    misc_deregister(&dma_data_misc);
	printk ("<0>dma_data module is unloaded. \n");
}
module_init(dma_data_init);
module_exit(dma_data_exit);
MODULE_LICENSE("GPL");
