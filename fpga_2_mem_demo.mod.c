#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc8b6343c, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xe4e30be6, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x1db1918f, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x8756075b, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x9a41fe09, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x516d626d, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x788fe103, __VMLINUX_SYMBOL_STR(iomem_resource) },
	{ 0x5f754e5a, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xfb961d14, __VMLINUX_SYMBOL_STR(__arm_ioremap) },
	{ 0xadf42bd5, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x4013b62f, __VMLINUX_SYMBOL_STR(__dma_request_channel) },
	{ 0x676bbc0f, __VMLINUX_SYMBOL_STR(_set_bit) },
	{ 0xaf8e8fef, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x5b8a112a, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x9e34fa8c, __VMLINUX_SYMBOL_STR(wait_for_completion) },
	{ 0xc2165d85, __VMLINUX_SYMBOL_STR(__arm_iounmap) },
	{ 0x5552d48d, __VMLINUX_SYMBOL_STR(dma_release_channel) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "7FE6CA43D69319AD0564BB5");
