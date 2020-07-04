/* ntl-br-entry.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_BR_ENTRY_H__
#define __NTL_BR_ENTRY_H__

#include <linux/debugfs.h>

#include "ntl-entry.h"

int ntl_br_entry_init(struct dentry *dentry);
void ntl_br_entry_exit(void);

#endif
