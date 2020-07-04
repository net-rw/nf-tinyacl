/* ntl-init.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/debugfs.h>

#include "ntl-priv.h"
#include "ntl-bridge-init.h"

static struct dentry *ntl_dentry;

static int __init ntl_init(void)
{
	int ret = -1;

	ntl_debug("NTL Core Module Init");

	ntl_dentry = debugfs_create_dir("ntl", NULL);
	if (!ntl_dentry) {
		ntl_debug("Failed to create top level ntl directory in debugfs");
		goto out;
	}

	ret = ntl_bridge_init(ntl_dentry);
	if (0 != ret)
		goto err_bridge_init;

	ret = 0;

out:
	return ret;

	/* errors */
err_bridge_init:
	debugfs_remove_recursive(ntl_dentry);
	goto out;
}

static void __exit ntl_exit(void)
{
	ntl_debug("NTL Core Module Exit");

	if (ntl_dentry) {
		ntl_debug("Remove ntl debugfs");
		debugfs_remove_recursive(ntl_dentry);
	}

	ntl_bridge_exit();

	ntl_debug("NTL Core Module Exit Complete");
}

module_init(ntl_init)
module_exit(ntl_exit)

MODULE_DESCRIPTION("NTL Core Module");
MODULE_AUTHOR("Leesoo Ahn <yisooan@fedoraproject.org>");
MODULE_LICENSE("Dual BSD/GPL");
