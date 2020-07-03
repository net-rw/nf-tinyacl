/* ntl-priv.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_PRIV_H__
#define __NTL_PRIV_H__

#define ntl_fmt(fmt, ...) \
	printk("NTL [%s:%d]: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#if defined(NTL_DEBUG_ENABLE)
# define __ntl_dbg(fmt, ...) ntl_fmt(fmt, ##__VA_ARGS__)
#else
# define __ntl_dbg(fmt, ...)
#endif

#define ntl_debug(fmt, ...) __ntl_dbg(fmt, ##__VA_ARGS__)
#define ntl_info(fmt, ...) ntl_fmt(fmt, ##__VA_ARGS__)

#endif
