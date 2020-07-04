/* ntl-nf-bridge.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_NF_BRIDGE_H__
#define __NTL_NF_BRIDGE_H__

#include <linux/debugfs.h>

int ntl_nf_bridge_init(struct dentry *dentry);
void ntl_nf_bridge_exit(void);

#endif
