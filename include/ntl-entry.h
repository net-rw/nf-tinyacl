/* ntl-entry.h
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#ifndef __NTL_ENTRY_H__
#define __NTL_ENTRY_H__

#include <linux/hash.h>
#include <linux/spinlock.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/debugfs.h>

#define NTL_ENTRY_UNFLAG   (0)
#define NTL_ENTRY_ALLOW_FG (1 << 0)
#define NTL_ENTRY_DENY_FG  (1 << 1)

struct acl_entry {
	/* attrs */
	u32 flags;
	u8 macaddr[ETH_ALEN];

	/* locks */
	spinlock_t lock;
	struct rcu_head rcu;

	/* mgmt */
	struct hlist_node hash_node;
	u64 key;
};

struct acl_entry * ntl_entry_lookup_by_key(u64 key);
struct acl_entry * ntl_entry_lookup_by_mac(u8 *macaddr);
int ntl_entry_new_acl_entry(u8 *macaddr, struct acl_entry **out_new_entry);
int ntl_entry_free_acl_entry(struct acl_entry *entry);

void ntl_entry_hash_add(struct acl_entry *entry);
void ntl_entry_hash_del(struct acl_entry *entry);
void ntl_entry_hash_iter(int (*callback)(void *));

#endif
