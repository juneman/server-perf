#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0x8707ca59, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x72df2f2a, __VMLINUX_SYMBOL_STR(up_read) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x15692c87, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x5ed439cb, __VMLINUX_SYMBOL_STR(dev_get_by_name) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xd0f0d945, __VMLINUX_SYMBOL_STR(down_read) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0xd6b59b5e, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0x4e0d355e, __VMLINUX_SYMBOL_STR(netif_rx) },
	{ 0x68dfc59f, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x98a75e25, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0x2bc95bd4, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xf97456ea, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x37befc70, __VMLINUX_SYMBOL_STR(jiffies_to_msecs) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x7c1372e8, __VMLINUX_SYMBOL_STR(panic) },
	{ 0x2da418b5, __VMLINUX_SYMBOL_STR(copy_to_user) },
	{ 0xb6ed1e53, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0x6c2e3320, __VMLINUX_SYMBOL_STR(strncmp) },
	{ 0xdd1a2871, __VMLINUX_SYMBOL_STR(down) },
	{ 0xbc1afedf, __VMLINUX_SYMBOL_STR(up_write) },
	{ 0xb8ab511d, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0x61b5ade0, __VMLINUX_SYMBOL_STR(down_write) },
	{ 0x6318c2ca, __VMLINUX_SYMBOL_STR(dev_kfree_skb_any) },
	{ 0xf3f9e89f, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0x93fca811, __VMLINUX_SYMBOL_STR(__get_free_pages) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x8bfac587, __VMLINUX_SYMBOL_STR(eth_type_trans) },
	{ 0x24f169fc, __VMLINUX_SYMBOL_STR(down_read_trylock) },
	{ 0x21fb443e, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x4302d0eb, __VMLINUX_SYMBOL_STR(free_pages) },
	{ 0xe45f60d8, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xc5cba27a, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x2e60bace, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xc4554217, __VMLINUX_SYMBOL_STR(up) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x33d169c9, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x5660b977, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0x657879ce, __VMLINUX_SYMBOL_STR(__init_rwsem) },
	{ 0xc73d5ab5, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

