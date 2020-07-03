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


/* --- Initialization--- */
enum {
	CMD_ADD = 1,
	CMD_DEL
};

/**
 * I know this is ugly, but no spend time
 * a lot to find out buggy exceptions.
 */
static int
parse_buf(unsigned char *buf, int buflen,
          unsigned int *out_cmd,
          unsigned char *out_mac, size_t maclen)
{
	int ret = -EINVAL;

	/* Not allowed parsing inputs same time */
	if (out_cmd && out_mac)
		goto out;
	/* Check input buf */
	if (!buf || buflen < 3)
		goto out;

	if (out_cmd) {
		if (memcmp(buf, "add", 3) == 0)
			*out_cmd = ret = CMD_ADD;
		else if (memcmp(buf, "del", 3) == 0)
			*out_cmd = ret = CMD_DEL;
		else
			goto out;
	}

	if (out_mac && maclen >= 6) {
		ret = sscanf(buf, "%2x:%2x:%2x:%2x:%2x:%2x",
		                  out_mac, out_mac+1, out_mac+2,
		                  out_mac+3, out_mac+4, out_mac+5);
		if (ret != 6) {
			ret = -EINVAL;
			goto out;
		}
	}

out:
	return ret;
}

#define BUF_MAX 256

static ssize_t ntl_entry_deny_list_add_del(struct file *file,
		const char __user *user_buf,
		size_t sz,
		loff_t *ppos)
{
	char dummy;
	int ret, cmd, has = 0;
	u8 buf[BUF_MAX] = { 0, },
	   buf_cmd[4] = { 0, },
	   buf_mac[32] = { 0, };
	unsigned char mac[ETH_ALEN] = { 0, };
	struct acl_entry *entry = NULL;

	if (sz > BUF_MAX)
		return -EINVAL;

	if (copy_from_user(buf, user_buf, sz) != 0)
		return -EFAULT;
	buf[sz-1] = '\0';

	ret = sscanf(buf, "%3s %32s%c", buf_cmd, buf_mac, &dummy);
	if (ret != 2) {
		ntl_debug("usage: echo \"<cmd> <mac>\"\n"
		          "cmd: add\n"
		          "     del\n"
		          "mac: XX:XX:XX:XX:XX:XX\n");
		return -EINVAL;
	}
	buf_cmd[sizeof(buf_cmd)-1] = '\0';
	buf_mac[sizeof(buf_mac)-1] = '\0';

	if (parse_buf(buf_cmd, sizeof(buf_cmd), &cmd, NULL, 0) < 0) {
		ntl_debug("Error on parsing cmd");
		return -EINVAL;
	}

	if (parse_buf(buf_mac, sizeof(buf_mac), NULL, mac, sizeof(mac)) < 0) {
		ntl_debug("Error on parsing mac addr");
		return -EINVAL;
	}

	switch (cmd) {
	case CMD_ADD:
		rcu_read_lock();
		if (ntl_entry_lookup_by_mac(mac))
			has = 1;
		rcu_read_unlock();
		if (has)
			break;

		if (ntl_entry_new_acl_entry(mac, &entry) != 0)
			return -ENOMEM;
		ntl_entry_hash_add(entry);
		break;
	case CMD_DEL:
		rcu_read_lock();
		entry = ntl_entry_lookup_by_mac(mac);
		if (entry)
			has = 1;
		rcu_read_unlock();
		if (!has)
			break;

		ntl_entry_hash_del(entry);
		break;
	default:
		return -EINVAL;
	}

	return sz;
}

static ssize_t ntl_entry_deny_list_show(struct file *file,
		char __user *user_buf,
		size_t sz,
		loff_t *ppos)
{
	int i;
	struct acl_entry *entry;

	rcu_read_lock();
	hash_for_each_rcu(entry_hash, i, entry, hash_node) {
		spin_lock(&entry->lock);
		ntl_info("mac [%s] to key [%llu]", mac2str(entry->macaddr), entry->key);
		spin_unlock(&entry->lock);
	}
	rcu_read_unlock();

	return 0;
}

static struct file_operations ntl_entry_deny_list_fops = {
	.write = ntl_entry_deny_list_add_del,
	.read = ntl_entry_deny_list_show,
};

int
ntl_entry_init(struct dentry *dentry)
{
	int ret = 0;

	ntl_debug("NTL Entry Init");

	if (!dentry) {
		ntl_debug("Parameter is NULL");
		ret = -EINVAL;
		goto out;
	}

	if (!debugfs_create_file("deny_list", S_IRUGO | S_IWUSR, dentry,
				NULL, &ntl_entry_deny_list_fops)) {
		ntl_debug("Failed to create ntl entry deny_list file in debugfs");
		ret = -1;
		goto out;
	}

out:
	return ret;
}
EXPORT_SYMBOL(ntl_entry_init);

void
ntl_entry_exit(void)
{
	ntl_debug("NTL Entry Exit");
}
EXPORT_SYMBOL(ntl_entry_exit);
