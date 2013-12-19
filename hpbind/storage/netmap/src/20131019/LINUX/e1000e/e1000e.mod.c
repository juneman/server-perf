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
	{ 0x3ce4ca6f, __VMLINUX_SYMBOL_STR(disable_irq) },
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0x5216b1ad, __VMLINUX_SYMBOL_STR(netdev_info) },
	{ 0x255cd20a, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x957e39b8, __VMLINUX_SYMBOL_STR(ethtool_op_get_ts_info) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xc4dc87, __VMLINUX_SYMBOL_STR(timecounter_init) },
	{ 0x704ee1c9, __VMLINUX_SYMBOL_STR(__pm_runtime_idle) },
	{ 0xc89caed2, __VMLINUX_SYMBOL_STR(netmap_attach) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x15692c87, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x91eb9b4, __VMLINUX_SYMBOL_STR(round_jiffies) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x94acbcb4, __VMLINUX_SYMBOL_STR(skb_pad) },
	{ 0x5a1d8686, __VMLINUX_SYMBOL_STR(page_address) },
	{ 0x8964b18c, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x9288a255, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0x4f12c0c0, __VMLINUX_SYMBOL_STR(napi_complete) },
	{ 0x50ffa89a, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xc7a4fbed, __VMLINUX_SYMBOL_STR(rtnl_lock) },
	{ 0x19708b34, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0x4ea25709, __VMLINUX_SYMBOL_STR(dql_reset) },
	{ 0x409b40de, __VMLINUX_SYMBOL_STR(netmap_rx_irq) },
	{ 0x7aa802ab, __VMLINUX_SYMBOL_STR(netif_carrier_on) },
	{ 0x48ca8947, __VMLINUX_SYMBOL_STR(pm_qos_add_request) },
	{ 0x67643d26, __VMLINUX_SYMBOL_STR(pm_qos_remove_request) },
	{ 0xc0a3d105, __VMLINUX_SYMBOL_STR(find_next_bit) },
	{ 0x6b06fdce, __VMLINUX_SYMBOL_STR(delayed_work_timer_fn) },
	{ 0x6bad7e2c, __VMLINUX_SYMBOL_STR(netif_carrier_off) },
	{ 0x4205ad24, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0x8e43289c, __VMLINUX_SYMBOL_STR(x86_dma_fallback_dev) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0x593a99b, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0x24d7b4eb, __VMLINUX_SYMBOL_STR(cancel_delayed_work_sync) },
	{ 0x42eccd85, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0xab04d62, __VMLINUX_SYMBOL_STR(__pm_runtime_resume) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x2a746631, __VMLINUX_SYMBOL_STR(pci_bus_write_config_word) },
	{ 0x2447533c, __VMLINUX_SYMBOL_STR(ktime_get_real) },
	{ 0x67dcbbc7, __VMLINUX_SYMBOL_STR(pci_disable_link_state_locked) },
	{ 0x2909f441, __VMLINUX_SYMBOL_STR(__alloc_pages_nodemask) },
	{ 0xc499ae1e, __VMLINUX_SYMBOL_STR(kstrdup) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x42590ec6, __VMLINUX_SYMBOL_STR(__dynamic_netdev_dbg) },
	{ 0x3375b6bb, __VMLINUX_SYMBOL_STR(skb_trim) },
	{ 0xf399ce75, __VMLINUX_SYMBOL_STR(netmap_no_pendintr) },
	{ 0xd6b59b5e, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0x7de09a51, __VMLINUX_SYMBOL_STR(__pskb_pull_tail) },
	{ 0xf6e403b3, __VMLINUX_SYMBOL_STR(ptp_clock_unregister) },
	{ 0xfb37da59, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xfd905c78, __VMLINUX_SYMBOL_STR(kmap_atomic) },
	{ 0xd5f2172f, __VMLINUX_SYMBOL_STR(del_timer_sync) },
	{ 0x2bc95bd4, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x98a65eae, __VMLINUX_SYMBOL_STR(pci_enable_pcie_error_reporting) },
	{ 0xaeb46116, __VMLINUX_SYMBOL_STR(pci_enable_msix) },
	{ 0x880733, __VMLINUX_SYMBOL_STR(pci_restore_state) },
	{ 0x396b65a9, __VMLINUX_SYMBOL_STR(netmap_ring_reinit) },
	{ 0xdb4d8c28, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xf97456ea, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x625ed5b7, __VMLINUX_SYMBOL_STR(ethtool_op_get_link) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xa00aca2a, __VMLINUX_SYMBOL_STR(dql_completed) },
	{ 0x8e5455e9, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x2da418b5, __VMLINUX_SYMBOL_STR(copy_to_user) },
	{ 0x7483272b, __VMLINUX_SYMBOL_STR(register_netdev) },
	{ 0x73e20c1c, __VMLINUX_SYMBOL_STR(strlcpy) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x1074b72, __VMLINUX_SYMBOL_STR(__pci_enable_wake) },
	{ 0x29154e1c, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x74a436c4, __VMLINUX_SYMBOL_STR(mem_section) },
	{ 0xa34f1ef5, __VMLINUX_SYMBOL_STR(crc32_le) },
	{ 0xed93f29e, __VMLINUX_SYMBOL_STR(__kunmap_atomic) },
	{ 0x270f1fa, __VMLINUX_SYMBOL_STR(dev_close) },
	{ 0xae6242bf, __VMLINUX_SYMBOL_STR(netmap_detach) },
	{ 0x8834396c, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x1902adf, __VMLINUX_SYMBOL_STR(netpoll_trap) },
	{ 0x73d6f30d, __VMLINUX_SYMBOL_STR(netif_napi_add) },
	{ 0x614b631c, __VMLINUX_SYMBOL_STR(dma_release_from_coherent) },
	{ 0x859c3728, __VMLINUX_SYMBOL_STR(ptp_clock_register) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x1a587054, __VMLINUX_SYMBOL_STR(device_wakeup_enable) },
	{ 0xb7817124, __VMLINUX_SYMBOL_STR(netmap_buffer_lut) },
	{ 0x6318c2ca, __VMLINUX_SYMBOL_STR(dev_kfree_skb_any) },
	{ 0x622ad674, __VMLINUX_SYMBOL_STR(contig_page_data) },
	{ 0xb3c8a662, __VMLINUX_SYMBOL_STR(netmap_disable_all_rings) },
	{ 0x27ac4d59, __VMLINUX_SYMBOL_STR(dma_alloc_from_coherent) },
	{ 0xe83d8f4c, __VMLINUX_SYMBOL_STR(dev_open) },
	{ 0xe523ad75, __VMLINUX_SYMBOL_STR(synchronize_irq) },
	{ 0x74954462, __VMLINUX_SYMBOL_STR(timecounter_read) },
	{ 0x5d95079e, __VMLINUX_SYMBOL_STR(dev_notice) },
	{ 0x5982c59, __VMLINUX_SYMBOL_STR(netmap_total_buffers) },
	{ 0x74990736, __VMLINUX_SYMBOL_STR(dev_kfree_skb_irq) },
	{ 0x4059792f, __VMLINUX_SYMBOL_STR(print_hex_dump) },
	{ 0x965a725d, __VMLINUX_SYMBOL_STR(pci_select_bars) },
	{ 0xc0bf6ead, __VMLINUX_SYMBOL_STR(timecounter_cyc2time) },
	{ 0x624ce178, __VMLINUX_SYMBOL_STR(netif_device_attach) },
	{ 0xd8f2cf7, __VMLINUX_SYMBOL_STR(napi_gro_receive) },
	{ 0x785b769, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x40a9b349, __VMLINUX_SYMBOL_STR(vzalloc) },
	{ 0x5569dcb5, __VMLINUX_SYMBOL_STR(netif_device_detach) },
	{ 0xf7ad1927, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x12a38747, __VMLINUX_SYMBOL_STR(usleep_range) },
	{ 0xeb6faaf3, __VMLINUX_SYMBOL_STR(pci_bus_read_config_word) },
	{ 0x6123b61a, __VMLINUX_SYMBOL_STR(__napi_schedule) },
	{ 0xe851bb05, __VMLINUX_SYMBOL_STR(queue_delayed_work_on) },
	{ 0xc7caed8c, __VMLINUX_SYMBOL_STR(pci_cleanup_aer_uncorrect_error_status) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x4334a4b5, __VMLINUX_SYMBOL_STR(pm_schedule_suspend) },
	{ 0x8bfac587, __VMLINUX_SYMBOL_STR(eth_type_trans) },
	{ 0x1fbd2292, __VMLINUX_SYMBOL_STR(netmap_buf_size) },
	{ 0xb44011e3, __VMLINUX_SYMBOL_STR(pskb_expand_head) },
	{ 0x54c146d5, __VMLINUX_SYMBOL_STR(netdev_err) },
	{ 0x15fd14a7, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xcc5005fe, __VMLINUX_SYMBOL_STR(msleep_interruptible) },
	{ 0xc0099c3d, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x67f7403e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x106148e9, __VMLINUX_SYMBOL_STR(netmap_buffer_base) },
	{ 0x21fb443e, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xf6ebc03b, __VMLINUX_SYMBOL_STR(net_ratelimit) },
	{ 0x2eed7c9c, __VMLINUX_SYMBOL_STR(netdev_warn) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0xcfde160a, __VMLINUX_SYMBOL_STR(eth_validate_addr) },
	{ 0xe2613a4b, __VMLINUX_SYMBOL_STR(pci_disable_pcie_error_reporting) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x2e60bace, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xbd5251bf, __VMLINUX_SYMBOL_STR(___pskb_trim) },
	{ 0x4845c423, __VMLINUX_SYMBOL_STR(param_array_ops) },
	{ 0x88f4e361, __VMLINUX_SYMBOL_STR(ptp_clock_index) },
	{ 0x2f7a69f5, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0xa1c394fd, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x999ce6b2, __VMLINUX_SYMBOL_STR(pci_prepare_to_sleep) },
	{ 0xf7a69cfe, __VMLINUX_SYMBOL_STR(netmap_reset) },
	{ 0xb6858634, __VMLINUX_SYMBOL_STR(pci_dev_run_wake) },
	{ 0xbcc155e8, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x8effebd3, __VMLINUX_SYMBOL_STR(pm_qos_update_request) },
	{ 0xd5fe90e0, __VMLINUX_SYMBOL_STR(put_page) },
	{ 0xb352177e, __VMLINUX_SYMBOL_STR(find_first_bit) },
	{ 0xf3616a10, __VMLINUX_SYMBOL_STR(dev_warn) },
	{ 0x3538da98, __VMLINUX_SYMBOL_STR(unregister_netdev) },
	{ 0xb2d48a2e, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x50430095, __VMLINUX_SYMBOL_STR(pci_enable_msi_block) },
	{ 0xca82e74d, __VMLINUX_SYMBOL_STR(__netif_schedule) },
	{ 0xc37f135e, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0x840865ff, __VMLINUX_SYMBOL_STR(pci_enable_device_mem) },
	{ 0x58f7f3c5, __VMLINUX_SYMBOL_STR(netmap_enable_all_rings) },
	{ 0xeb43383b, __VMLINUX_SYMBOL_STR(skb_tstamp_tx) },
	{ 0x7c483126, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0xc5394d1f, __VMLINUX_SYMBOL_STR(pci_release_selected_regions) },
	{ 0x33d169c9, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x6d044c26, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x319b9478, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0xe4290419, __VMLINUX_SYMBOL_STR(pcie_capability_write_word) },
	{ 0x6e720ff2, __VMLINUX_SYMBOL_STR(rtnl_unlock) },
	{ 0x9e7d6bd0, __VMLINUX_SYMBOL_STR(__udelay) },
	{ 0x3c607846, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x8c036aff, __VMLINUX_SYMBOL_STR(pci_request_selected_regions_exclusive) },
	{ 0xc0f5d787, __VMLINUX_SYMBOL_STR(device_set_wakeup_enable) },
	{ 0x94ad147b, __VMLINUX_SYMBOL_STR(pcie_capability_read_word) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x4d5430de, __VMLINUX_SYMBOL_STR(pci_save_state) },
	{ 0xad89e21, __VMLINUX_SYMBOL_STR(alloc_etherdev_mqs) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=netmap_lin,ptp";

MODULE_ALIAS("pci:v00008086d0000105Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000105Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010A4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010BCsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010A5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001060sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010D9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010DAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010D5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010B9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000107Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000108Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000108Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000109Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010D3sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001096sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010BAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001098sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010BBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000104Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000104Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000104Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000104Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001049sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001501sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C0sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C2sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C3sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010BDsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000294Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010E5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010BFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F5sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010CBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010CCsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010CDsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010CEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010DEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010DFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001525sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010EAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010EBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010EFsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F0sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001502sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001503sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000153Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000153Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000155Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001559sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "807AFE1A89D409151B180C0");
