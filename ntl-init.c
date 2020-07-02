/* ntl-init.c
 *
 * Copyright (c) 2020, Leesoo <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/debugfs.h>

#include "ntl-priv.h"
#include "ntl-entry.h"
#include "ntl-nf-bridge.h"

static struct dentry *ntl_dentry;

static int __init ntl_init(void)
{
	int ret = -1;

	ntl_debug("NTL Module Init");

	ntl_dentry = debugfs_create_dir("ntl", NULL);
	if (!ntl_dentry) {
		ntl_debug("Failed to create ntl directory in debugfs");
		goto out;
	}

	ret = ntl_entry_init(ntl_dentry);
	if (0 != ret)
		goto err_entry;

	ret = ntl_nf_bridge_init(ntl_dentry);
	if (0 != ret)
		goto err_nf_bridge;

	ret = 0;

out:
	return ret;

	/* errors */
err_nf_bridge:
	ntl_entry_exit();
err_entry:
	debugfs_remove_recursive(ntl_dentry);
	goto out;
}

static void __exit ntl_exit(void)
{
	ntl_debug("NTL Module Exit");

	if (ntl_dentry) {
		ntl_debug("Remove ntl debugfs");
		debugfs_remove_recursive(ntl_dentry);
	}

	/* Must be called in order */
	ntl_nf_bridge_exit();
	ntl_entry_exit();

	ntl_debug("NTL Module Exit Complete");
}

module_init(ntl_init)
module_exit(ntl_exit)

MODULE_DESCRIPTION("NTL Core Module");
MODULE_AUTHOR("Leesoo Ahn <yisooan@fedoraproject.org>");
MODULE_LICENSE("Dual BSD/GPL");
