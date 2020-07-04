/* ntl-entry.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include <linux/string.h>
#include <linux/if_ether.h>
#include <linux/hashtable.h>

#include "ntl-priv.h"
#include "ntl-util.h"
#include "ntl-entry.h"

static DEFINE_HASHTABLE(entry_hash, 4);

/**
 * must be called under rcu_read_lock()
 */
struct acl_entry *
ntl_entry_lookup_by_key(u64 key)
{
	struct acl_entry *entry; 

	hlist_for_each_entry_rcu(entry,
			                 &entry_hash[hash_min(key, HASH_BITS(entry_hash))],
							 hash_node)
	{
		if (entry->key == key)
			return entry;
	}

	return NULL;
}

/**
 * must be called under rcu_read_lock()
 */
struct acl_entry *
ntl_entry_lookup_by_mac(u8 *macaddr)
{
	u64 key;

	if (!macaddr)
		return NULL;

	key = mac_to_u64(macaddr);
	return ntl_entry_lookup_by_key(key);
}

/**
 * macaddr: only 6 bytes macaddr
 */
int
ntl_entry_new_acl_entry(u8 *macaddr, struct acl_entry **out_new_entry)
{
	struct acl_entry *entry;

	/* macaddr is NULL or *out_new_entry is not NULL */
	if (!macaddr || !out_new_entry || *out_new_entry)
		return -EINVAL;

	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	memset(entry, 0, sizeof(*entry));
	entry->key = mac_to_u64(macaddr);
	memcpy(entry->macaddr, macaddr, sizeof(entry->macaddr));
	spin_lock_init(&entry->lock);
	
	*out_new_entry = entry;

	return 0;
}

/**
 * NOTE be carefull call this directly, because
 * entry_reclaim could be in queue while this in process.
 * In that case, dereferencing dangling pointer happens.
 */
int
ntl_entry_free_acl_entry(struct acl_entry *entry)
{
	if (!entry)
		return -EINVAL;

	/* cleanup private attrs here */

	kfree(entry);

	return 0;
}

void
ntl_entry_hash_add(struct acl_entry *entry)
{
	if (!entry)
		return;

	ntl_debug("add - mac [%s] to key [%llu]", mac2str(entry->macaddr), entry->key);

	spin_lock(&entry->lock);
	hash_add_rcu(entry_hash, &entry->hash_node, entry->key);
	spin_unlock(&entry->lock);
}

static void
entry_reclaim(struct rcu_head *rp)
{
	struct acl_entry *entry = container_of(rp, struct acl_entry, rcu);

	ntl_entry_free_acl_entry(entry);
}

void
ntl_entry_hash_del(struct acl_entry *entry)
{
	if (!entry)
		return;

	ntl_debug("del - mac [%s] to key [%llu]", mac2str(entry->macaddr), entry->key);

	spin_lock(&entry->lock);
	hash_del_rcu(&entry->hash_node);
	spin_unlock(&entry->lock);
	call_rcu(&entry->rcu, entry_reclaim);
}

/**
 * must be called under rcu_read_lock()
 */
void
ntl_entry_hash_iter(int (*callback)(void *))
{
	int i;
	struct acl_entry *entry;

	if (!callback)
		return;

	hash_for_each_rcu(entry_hash, i, entry, hash_node) {
		callback(entry);
	}
}
