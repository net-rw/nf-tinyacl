/* ntl-util.h
 *
 * Copyright (c) 2020, Leesoo <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_UTIL_H__
#define __NTL_UTIL_H__

#include <linux/types.h>

unsigned char * mac2str(u8 *macaddr);
u64 mac_to_u64(const u8 *macaddr);

#endif
