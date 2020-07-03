/* ntl-util.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include "ntl-util.h"

#define IBUFSZ 128

extern int snprintf(char *buf, size_t size, const char *fmt, ...);

unsigned char *
mac2str(u8 *macaddr)
{
	int result;
	static unsigned char buf[IBUFSZ] = { 0, };

	if (!macaddr)
		return buf;

	result = snprintf(buf, IBUFSZ, "%02X:%02X:%02X:%02X:%02X:%02X",
	                               macaddr[0], macaddr[1], macaddr[2],
	                               macaddr[3], macaddr[4], macaddr[5]);
	(void)result;

	return buf;
}

/**
 * macaddr: only 6 bytes macaddr
 */
u64
mac_to_u64(const u8 *macaddr)
{
	u64 val = 0;
	const u8 *ptr;
	int i;

	if (!macaddr)
		goto ret;

	ptr = macaddr;

	for (i = 5; i >= 0; ++ptr, i--) {
		val |= (u64)*ptr << (8 * i);
	}

ret:
	return val;
}
