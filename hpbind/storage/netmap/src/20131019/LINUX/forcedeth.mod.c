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
	{ 0x957e39b8, __VMLINUX_SYMBOL_STR(ethtool_op_get_ts_info) },
	{ 0x625ed5b7, __VMLINUX_SYMBOL_STR(ethtool_op_get_link) },
	{ 0xcfde160a, __VMLINUX_SYMBOL_STR(eth_validate_addr) },
	{ 0x15692c87, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x1976aa06, __VMLINUX_SYMBOL_STR(param_ops_bool) },
	{ 0xd0ee38b8, __VMLINUX_SYMBOL_STR(schedule_timeout_uninterruptible) },
	{ 0xa1c394fd, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x9288a255, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0x465593c5, __VMLINUX_SYMBOL_STR(__dynamic_dev_dbg) },
	{ 0xdb4d8c28, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x785b769, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x7483272b, __VMLINUX_SYMBOL_STR(register_netdev) },
	{ 0x73d6f30d, __VMLINUX_SYMBOL_STR(netif_napi_add) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0xfb146997, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0xfb37da59, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x39e4ac23, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x593a99b, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0xad89e21, __VMLINUX_SYMBOL_STR(alloc_etherdev_mqs) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x8964b18c, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x8e5455e9, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0x5c38bcc8, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0xae6242bf, __VMLINUX_SYMBOL_STR(netmap_detach) },
	{ 0x3538da98, __VMLINUX_SYMBOL_STR(unregister_netdev) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x2bc95bd4, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x1d858c11, __VMLINUX_SYMBOL_STR(pci_set_power_state) },
	{ 0xa6f27e16, __VMLINUX_SYMBOL_STR(pci_wake_from_d3) },
	{ 0x50ffa89a, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xa8721b97, __VMLINUX_SYMBOL_STR(system_state) },
	{ 0x5569dcb5, __VMLINUX_SYMBOL_STR(netif_device_detach) },
	{ 0xd5f2172f, __VMLINUX_SYMBOL_STR(del_timer_sync) },
	{ 0xe523ad75, __VMLINUX_SYMBOL_STR(synchronize_irq) },
	{ 0xeb43383b, __VMLINUX_SYMBOL_STR(skb_tstamp_tx) },
	{ 0x5a1d8686, __VMLINUX_SYMBOL_STR(page_address) },
	{ 0x4f12c0c0, __VMLINUX_SYMBOL_STR(napi_complete) },
	{ 0x6741a31, __VMLINUX_SYMBOL_STR(pci_bus_write_config_dword) },
	{ 0x624ce178, __VMLINUX_SYMBOL_STR(netif_device_attach) },
	{ 0x319b9478, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0xf7a69cfe, __VMLINUX_SYMBOL_STR(netmap_reset) },
	{ 0xaaedec18, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x74a436c4, __VMLINUX_SYMBOL_STR(mem_section) },
	{ 0xd6b59b5e, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0x2eed7c9c, __VMLINUX_SYMBOL_STR(netdev_warn) },
	{ 0x409b40de, __VMLINUX_SYMBOL_STR(netmap_rx_irq) },
	{ 0x42590ec6, __VMLINUX_SYMBOL_STR(__dynamic_netdev_dbg) },
	{ 0xd8f2cf7, __VMLINUX_SYMBOL_STR(napi_gro_receive) },
	{ 0x8bfac587, __VMLINUX_SYMBOL_STR(eth_type_trans) },
	{ 0x7c483126, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0xc37f135e, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0x7aa802ab, __VMLINUX_SYMBOL_STR(netif_carrier_on) },
	{ 0xf97456ea, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x21fb443e, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xbcc155e8, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x8834396c, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x91eb9b4, __VMLINUX_SYMBOL_STR(round_jiffies) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x5383f34b, __VMLINUX_SYMBOL_STR(_raw_spin_trylock) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0x396b65a9, __VMLINUX_SYMBOL_STR(netmap_ring_reinit) },
	{ 0x5982c59, __VMLINUX_SYMBOL_STR(netmap_total_buffers) },
	{ 0x1fbd2292, __VMLINUX_SYMBOL_STR(netmap_buf_size) },
	{ 0x106148e9, __VMLINUX_SYMBOL_STR(netmap_buffer_base) },
	{ 0xb7817124, __VMLINUX_SYMBOL_STR(netmap_buffer_lut) },
	{ 0xc89caed2, __VMLINUX_SYMBOL_STR(netmap_attach) },
	{ 0x8e43289c, __VMLINUX_SYMBOL_STR(x86_dma_fallback_dev) },
	{ 0x27ac4d59, __VMLINUX_SYMBOL_STR(dma_alloc_from_coherent) },
	{ 0x73e20c1c, __VMLINUX_SYMBOL_STR(strlcpy) },
	{ 0x6bad7e2c, __VMLINUX_SYMBOL_STR(netif_carrier_off) },
	{ 0x799aca4, __VMLINUX_SYMBOL_STR(local_bh_enable) },
	{ 0x54efb5d6, __VMLINUX_SYMBOL_STR(cpu_number) },
	{ 0x3ff62317, __VMLINUX_SYMBOL_STR(local_bh_disable) },
	{ 0x8bf826c, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_bh) },
	{ 0xa4eb4eff, __VMLINUX_SYMBOL_STR(_raw_spin_lock_bh) },
	{ 0x1e047854, __VMLINUX_SYMBOL_STR(warn_slowpath_fmt) },
	{ 0x2e60bace, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xc0f5d787, __VMLINUX_SYMBOL_STR(device_set_wakeup_enable) },
	{ 0x614b631c, __VMLINUX_SYMBOL_STR(dma_release_from_coherent) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0x3ce4ca6f, __VMLINUX_SYMBOL_STR(disable_irq) },
	{ 0xf1faac3a, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irq) },
	{ 0x67f7403e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x6123b61a, __VMLINUX_SYMBOL_STR(__napi_schedule) },
	{ 0xa00aca2a, __VMLINUX_SYMBOL_STR(dql_completed) },
	{ 0xca82e74d, __VMLINUX_SYMBOL_STR(__netif_schedule) },
	{ 0x1902adf, __VMLINUX_SYMBOL_STR(netpoll_trap) },
	{ 0x6318c2ca, __VMLINUX_SYMBOL_STR(dev_kfree_skb_any) },
	{ 0x50430095, __VMLINUX_SYMBOL_STR(pci_enable_msi_block) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xaeb46116, __VMLINUX_SYMBOL_STR(pci_enable_msix) },
	{ 0x2f7a69f5, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0x19708b34, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x3c607846, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x4ea25709, __VMLINUX_SYMBOL_STR(dql_reset) },
	{ 0x79aa04a2, __VMLINUX_SYMBOL_STR(get_random_bytes) },
	{ 0x5216b1ad, __VMLINUX_SYMBOL_STR(netdev_info) },
	{ 0x12a38747, __VMLINUX_SYMBOL_STR(usleep_range) },
	{ 0x9e7d6bd0, __VMLINUX_SYMBOL_STR(__udelay) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x15fd14a7, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=netmap_lin";

MODULE_ALIAS("pci:v000010DEd000001C3sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000066sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000000D6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000086sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd0000008Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000000E6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000000DFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000056sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000057sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000037sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000268sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000269sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000372sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000373sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000003E5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000003E6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000003EEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000003EFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000450sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000451sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000452sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000453sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd0000054Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd0000054Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd0000054Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd0000054Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000007DCsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000007DDsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000007DEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd000007DFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000760sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000761sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000762sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000763sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000AB0sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000AB1sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000AB2sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000AB3sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010DEd00000D7Dsv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "6823E5145C50BC034A23E02");
