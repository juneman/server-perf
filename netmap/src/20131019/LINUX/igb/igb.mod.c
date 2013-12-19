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
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0x51f7dfa2, __VMLINUX_SYMBOL_STR(device_remove_file) },
	{ 0x5216b1ad, __VMLINUX_SYMBOL_STR(netdev_info) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xc4dc87, __VMLINUX_SYMBOL_STR(timecounter_init) },
	{ 0x704ee1c9, __VMLINUX_SYMBOL_STR(__pm_runtime_idle) },
	{ 0x498a2ddb, __VMLINUX_SYMBOL_STR(pci_enable_sriov) },
	{ 0xe6da44a, __VMLINUX_SYMBOL_STR(set_normalized_timespec) },
	{ 0xc89caed2, __VMLINUX_SYMBOL_STR(netmap_attach) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x8f80354c, __VMLINUX_SYMBOL_STR(i2c_smbus_read_byte_data) },
	{ 0x15692c87, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x91eb9b4, __VMLINUX_SYMBOL_STR(round_jiffies) },
	{ 0xed53eefd, __VMLINUX_SYMBOL_STR(pci_sriov_set_totalvfs) },
	{ 0x94acbcb4, __VMLINUX_SYMBOL_STR(skb_pad) },
	{ 0x5a1d8686, __VMLINUX_SYMBOL_STR(page_address) },
	{ 0x8964b18c, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0x9469482, __VMLINUX_SYMBOL_STR(kfree_call_rcu) },
	{ 0xda1ce447, __VMLINUX_SYMBOL_STR(cpu_online_mask) },
	{ 0x9288a255, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0x4f12c0c0, __VMLINUX_SYMBOL_STR(napi_complete) },
	{ 0x50ffa89a, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x79cf9956, __VMLINUX_SYMBOL_STR(i2c_smbus_write_byte_data) },
	{ 0xc7a4fbed, __VMLINUX_SYMBOL_STR(rtnl_lock) },
	{ 0x19708b34, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0x27f2c1cf, __VMLINUX_SYMBOL_STR(hwmon_device_unregister) },
	{ 0x4ea25709, __VMLINUX_SYMBOL_STR(dql_reset) },
	{ 0x409b40de, __VMLINUX_SYMBOL_STR(netmap_rx_irq) },
	{ 0x7aa802ab, __VMLINUX_SYMBOL_STR(netif_carrier_on) },
	{ 0xdf250ee0, __VMLINUX_SYMBOL_STR(pci_disable_sriov) },
	{ 0xc0a3d105, __VMLINUX_SYMBOL_STR(find_next_bit) },
	{ 0x6b06fdce, __VMLINUX_SYMBOL_STR(delayed_work_timer_fn) },
	{ 0x6bad7e2c, __VMLINUX_SYMBOL_STR(netif_carrier_off) },
	{ 0x4205ad24, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0xf087137d, __VMLINUX_SYMBOL_STR(__dynamic_pr_debug) },
	{ 0x8e43289c, __VMLINUX_SYMBOL_STR(x86_dma_fallback_dev) },
	{ 0xb1f5c8d4, __VMLINUX_SYMBOL_STR(driver_for_each_device) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0x593a99b, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0x24d7b4eb, __VMLINUX_SYMBOL_STR(cancel_delayed_work_sync) },
	{ 0xab04d62, __VMLINUX_SYMBOL_STR(__pm_runtime_resume) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x2447533c, __VMLINUX_SYMBOL_STR(ktime_get_real) },
	{ 0x54efb5d6, __VMLINUX_SYMBOL_STR(cpu_number) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xf4c91ed, __VMLINUX_SYMBOL_STR(ns_to_timespec) },
	{ 0x2909f441, __VMLINUX_SYMBOL_STR(__alloc_pages_nodemask) },
	{ 0xd08a868, __VMLINUX_SYMBOL_STR(netif_napi_del) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x42590ec6, __VMLINUX_SYMBOL_STR(__dynamic_netdev_dbg) },
	{ 0xf399ce75, __VMLINUX_SYMBOL_STR(netmap_no_pendintr) },
	{ 0xd6b59b5e, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0xf6e403b3, __VMLINUX_SYMBOL_STR(ptp_clock_unregister) },
	{ 0xfb37da59, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x1b0c76ea, __VMLINUX_SYMBOL_STR(dca3_get_tag) },
	{ 0xd5f2172f, __VMLINUX_SYMBOL_STR(del_timer_sync) },
	{ 0x2bc95bd4, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x98a65eae, __VMLINUX_SYMBOL_STR(pci_enable_pcie_error_reporting) },
	{ 0xac34ecec, __VMLINUX_SYMBOL_STR(dca_register_notify) },
	{ 0xaeb46116, __VMLINUX_SYMBOL_STR(pci_enable_msix) },
	{ 0x880733, __VMLINUX_SYMBOL_STR(pci_restore_state) },
	{ 0x1a33ab9, __VMLINUX_SYMBOL_STR(dca_unregister_notify) },
	{ 0x396b65a9, __VMLINUX_SYMBOL_STR(netmap_ring_reinit) },
	{ 0xdb4d8c28, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xf97456ea, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xa00aca2a, __VMLINUX_SYMBOL_STR(dql_completed) },
	{ 0xe4f5fe6, __VMLINUX_SYMBOL_STR(kunmap) },
	{ 0x8e5455e9, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0xb6ed1e53, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0x2da418b5, __VMLINUX_SYMBOL_STR(copy_to_user) },
	{ 0x7483272b, __VMLINUX_SYMBOL_STR(register_netdev) },
	{ 0x73e20c1c, __VMLINUX_SYMBOL_STR(strlcpy) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x1074b72, __VMLINUX_SYMBOL_STR(__pci_enable_wake) },
	{ 0x74a436c4, __VMLINUX_SYMBOL_STR(mem_section) },
	{ 0x270f1fa, __VMLINUX_SYMBOL_STR(dev_close) },
	{ 0x1ab0a87, __VMLINUX_SYMBOL_STR(netif_set_real_num_rx_queues) },
	{ 0xae6242bf, __VMLINUX_SYMBOL_STR(netmap_detach) },
	{ 0x8834396c, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x151d4d3d, __VMLINUX_SYMBOL_STR(netif_set_real_num_tx_queues) },
	{ 0x1902adf, __VMLINUX_SYMBOL_STR(netpoll_trap) },
	{ 0x73d6f30d, __VMLINUX_SYMBOL_STR(netif_napi_add) },
	{ 0x614b631c, __VMLINUX_SYMBOL_STR(dma_release_from_coherent) },
	{ 0x859c3728, __VMLINUX_SYMBOL_STR(ptp_clock_register) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x732207a7, __VMLINUX_SYMBOL_STR(dca_add_requester) },
	{ 0xb7817124, __VMLINUX_SYMBOL_STR(netmap_buffer_lut) },
	{ 0x6318c2ca, __VMLINUX_SYMBOL_STR(dev_kfree_skb_any) },
	{ 0x622ad674, __VMLINUX_SYMBOL_STR(contig_page_data) },
	{ 0xb3c8a662, __VMLINUX_SYMBOL_STR(netmap_disable_all_rings) },
	{ 0x27ac4d59, __VMLINUX_SYMBOL_STR(dma_alloc_from_coherent) },
	{ 0xe83d8f4c, __VMLINUX_SYMBOL_STR(dev_open) },
	{ 0xe523ad75, __VMLINUX_SYMBOL_STR(synchronize_irq) },
	{ 0x74954462, __VMLINUX_SYMBOL_STR(timecounter_read) },
	{ 0xc8834b63, __VMLINUX_SYMBOL_STR(device_create_file) },
	{ 0x5982c59, __VMLINUX_SYMBOL_STR(netmap_total_buffers) },
	{ 0x4059792f, __VMLINUX_SYMBOL_STR(print_hex_dump) },
	{ 0x965a725d, __VMLINUX_SYMBOL_STR(pci_select_bars) },
	{ 0xa46c1197, __VMLINUX_SYMBOL_STR(i2c_del_adapter) },
	{ 0xc0bf6ead, __VMLINUX_SYMBOL_STR(timecounter_cyc2time) },
	{ 0x624ce178, __VMLINUX_SYMBOL_STR(netif_device_attach) },
	{ 0xd8f2cf7, __VMLINUX_SYMBOL_STR(napi_gro_receive) },
	{ 0x785b769, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x40a9b349, __VMLINUX_SYMBOL_STR(vzalloc) },
	{ 0xe5d91ec5, __VMLINUX_SYMBOL_STR(__free_pages) },
	{ 0x5569dcb5, __VMLINUX_SYMBOL_STR(netif_device_detach) },
	{ 0xf7ad1927, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x12a38747, __VMLINUX_SYMBOL_STR(usleep_range) },
	{ 0xeb6faaf3, __VMLINUX_SYMBOL_STR(pci_bus_read_config_word) },
	{ 0x58db3538, __VMLINUX_SYMBOL_STR(kmap) },
	{ 0x6123b61a, __VMLINUX_SYMBOL_STR(__napi_schedule) },
	{ 0xe851bb05, __VMLINUX_SYMBOL_STR(queue_delayed_work_on) },
	{ 0xc7caed8c, __VMLINUX_SYMBOL_STR(pci_cleanup_aer_uncorrect_error_status) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xaaedec18, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x4334a4b5, __VMLINUX_SYMBOL_STR(pm_schedule_suspend) },
	{ 0x8bfac587, __VMLINUX_SYMBOL_STR(eth_type_trans) },
	{ 0x1fbd2292, __VMLINUX_SYMBOL_STR(netmap_buf_size) },
	{ 0xb44011e3, __VMLINUX_SYMBOL_STR(pskb_expand_head) },
	{ 0x54c146d5, __VMLINUX_SYMBOL_STR(netdev_err) },
	{ 0x15fd14a7, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xcc5005fe, __VMLINUX_SYMBOL_STR(msleep_interruptible) },
	{ 0x67f7403e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x106148e9, __VMLINUX_SYMBOL_STR(netmap_buffer_base) },
	{ 0x465593c5, __VMLINUX_SYMBOL_STR(__dynamic_dev_dbg) },
	{ 0x21fb443e, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xf6ebc03b, __VMLINUX_SYMBOL_STR(net_ratelimit) },
	{ 0x1d858c11, __VMLINUX_SYMBOL_STR(pci_set_power_state) },
	{ 0x2eed7c9c, __VMLINUX_SYMBOL_STR(netdev_warn) },
	{ 0x4f68e5c9, __VMLINUX_SYMBOL_STR(do_gettimeofday) },
	{ 0xcfde160a, __VMLINUX_SYMBOL_STR(eth_validate_addr) },
	{ 0x1e047854, __VMLINUX_SYMBOL_STR(warn_slowpath_fmt) },
	{ 0xe2613a4b, __VMLINUX_SYMBOL_STR(pci_disable_pcie_error_reporting) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x2e60bace, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x88f4e361, __VMLINUX_SYMBOL_STR(ptp_clock_index) },
	{ 0x2f7a69f5, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0xa1c394fd, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xe012490b, __VMLINUX_SYMBOL_STR(skb_add_rx_frag) },
	{ 0x31770c38, __VMLINUX_SYMBOL_STR(pci_num_vf) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x999ce6b2, __VMLINUX_SYMBOL_STR(pci_prepare_to_sleep) },
	{ 0xf7a69cfe, __VMLINUX_SYMBOL_STR(netmap_reset) },
	{ 0xbcc155e8, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0xa8721b97, __VMLINUX_SYMBOL_STR(system_state) },
	{ 0x74c134b9, __VMLINUX_SYMBOL_STR(__sw_hweight32) },
	{ 0xb352177e, __VMLINUX_SYMBOL_STR(find_first_bit) },
	{ 0xf3616a10, __VMLINUX_SYMBOL_STR(dev_warn) },
	{ 0x3538da98, __VMLINUX_SYMBOL_STR(unregister_netdev) },
	{ 0xc0f300fd, __VMLINUX_SYMBOL_STR(i2c_bit_add_bus) },
	{ 0xb2d48a2e, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x3f24167e, __VMLINUX_SYMBOL_STR(pci_vfs_assigned) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x50430095, __VMLINUX_SYMBOL_STR(pci_enable_msi_block) },
	{ 0xca82e74d, __VMLINUX_SYMBOL_STR(__netif_schedule) },
	{ 0xc37f135e, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0x1f5a43f7, __VMLINUX_SYMBOL_STR(dca_remove_requester) },
	{ 0x840865ff, __VMLINUX_SYMBOL_STR(pci_enable_device_mem) },
	{ 0x58f7f3c5, __VMLINUX_SYMBOL_STR(netmap_enable_all_rings) },
	{ 0xeb43383b, __VMLINUX_SYMBOL_STR(skb_tstamp_tx) },
	{ 0x7c483126, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0xa6f27e16, __VMLINUX_SYMBOL_STR(pci_wake_from_d3) },
	{ 0xc5394d1f, __VMLINUX_SYMBOL_STR(pci_release_selected_regions) },
	{ 0x1aa41506, __VMLINUX_SYMBOL_STR(pci_request_selected_regions) },
	{ 0x33d169c9, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x6d044c26, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x490cf8a8, __VMLINUX_SYMBOL_STR(skb_copy_bits) },
	{ 0xc834f4e5, __VMLINUX_SYMBOL_STR(i2c_new_device) },
	{ 0x319b9478, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0x435411e7, __VMLINUX_SYMBOL_STR(hwmon_device_register) },
	{ 0xe4290419, __VMLINUX_SYMBOL_STR(pcie_capability_write_word) },
	{ 0x6e720ff2, __VMLINUX_SYMBOL_STR(rtnl_unlock) },
	{ 0x9e7d6bd0, __VMLINUX_SYMBOL_STR(__udelay) },
	{ 0x3c607846, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x94ad147b, __VMLINUX_SYMBOL_STR(pcie_capability_read_word) },
	{ 0xc0f5d787, __VMLINUX_SYMBOL_STR(device_set_wakeup_enable) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x4d5430de, __VMLINUX_SYMBOL_STR(pci_save_state) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0xad89e21, __VMLINUX_SYMBOL_STR(alloc_etherdev_mqs) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=netmap_lin,i2c-core,ptp,dca,i2c-algo-bit";

MODULE_ALIAS("pci:v00008086d00001F40sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001F41sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001F45sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001539sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001533sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001536sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001537sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001538sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001521sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001522sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001523sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001524sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001527sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001510sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001511sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001516sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00000438sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000043Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000043Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00000440sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001518sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010E6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010E7sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001526sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010E8sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010A7sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010A9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010D6sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "C57CB5FA9FA35FE59570C15");
