/* ntl-br-entry.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_BR_ENTRY_H__
#define __NTL_BR_ENTRY_H__

#include "ntl-entry.h"

int ntl_br_entry_init(void *parent_dentry);
void ntl_br_entry_exit(void);

#endif
