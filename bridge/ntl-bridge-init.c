/* ntl-bridge-init.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include <linux/debugfs.h>

#include "ntl-priv.h"
#include "ntl-br-entry.h"
#include "ntl-nf-bridge.h"

static struct dentry *bridge_dentry;

int
ntl_bridge_init(struct dentry *dentry)
{
	int ret = -1;

	ntl_debug("NTL Bridge Module Init");

	if (!dentry) {
		ntl_debug("Parameter is NULL");
		ret = -EINVAL;
		goto out;
	}

	bridge_dentry = debugfs_create_dir("bridge", dentry);
	if (!bridge_dentry) {
		ntl_debug("Failed to create bridge directory in debugfs");
		goto out;
	}

	ret = ntl_br_entry_init(bridge_dentry);
	if (0 != ret)
		goto err_br_entry;

	ret = ntl_nf_bridge_init(bridge_dentry);
	if (0 != ret)
		goto err_nf_bridge;

	ret = 0;

out:
	return ret;

	/* errors */
err_nf_bridge:
	ntl_br_entry_exit();
err_br_entry:
	debugfs_remove_recursive(bridge_dentry);
	goto out;
}
EXPORT_SYMBOL(ntl_bridge_init);

void
ntl_bridge_exit(void)
{
	ntl_debug("NTL Bridge Module Exit");

	if (bridge_dentry) {
		ntl_debug("Remove ntl debugfs");
		debugfs_remove_recursive(bridge_dentry);
	}

	/* Must be called in order */
	ntl_nf_bridge_exit();
	ntl_br_entry_exit();

	ntl_debug("NTL Bridge Module Exit Complete");
}
EXPORT_SYMBOL(ntl_bridge_exit);
