/* ntl-bridge-init.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include "ntl-priv.h"
#include "ntl-procfs.h"
#include "ntl-br-entry.h"
#include "ntl-nf-bridge.h"

static struct ntl_fs_dentry *bridge_dentry;

int
ntl_bridge_init(void *parent_dentry)
{
	int ret = -1;

	ntl_debug("NTL Bridge Module Init");

	if (!parent_dentry) {
		ntl_debug("Parameter is NULL");
		ret = -EINVAL;
		goto out;
	}

	bridge_dentry = ntl_proc_mkdir("bridge", parent_dentry);
	if (!bridge_dentry) {
		ntl_debug("Failed to create bridge directory in fs");
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
	ntl_proc_remove(bridge_dentry);
	goto out;
}
EXPORT_SYMBOL(ntl_bridge_init);

void
ntl_bridge_exit(void)
{
	ntl_debug("NTL Bridge Module Exit");

	if (bridge_dentry) {
		ntl_debug("Remove ntl debugfs");
		ntl_proc_remove(bridge_dentry);
	}

	/* Must be called in order */
	ntl_nf_bridge_exit();
	ntl_br_entry_exit();

	ntl_debug("NTL Bridge Module Exit Complete");
}
EXPORT_SYMBOL(ntl_bridge_exit);
