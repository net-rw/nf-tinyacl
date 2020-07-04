/* ntl-br-entry.c
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
#include "ntl-br-entry.h"

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

static ssize_t
ntl_entry_deny_list_add_del(struct file *file,
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

static int
show_cb(void *ptr)
{
	struct acl_entry *entry;

	if (!ptr)
		return -EINVAL;

	entry = (struct acl_entry*)ptr;

	spin_lock(&entry->lock);
	ntl_debug("key [%llu] : mac [%s]", entry->key, mac2str(entry->macaddr));
	spin_unlock(&entry->lock);

	return 0;
}

static ssize_t
ntl_entry_deny_list_show(struct file *file,
		char __user *user_buf,
		size_t sz,
		loff_t *ppos)
{
	rcu_read_lock();
	ntl_entry_hash_iter(show_cb);
	rcu_read_unlock();

	return 0;
}

static struct file_operations ntl_entry_deny_list_fops = {
	.write = ntl_entry_deny_list_add_del,
	.read = ntl_entry_deny_list_show,
};

int
ntl_br_entry_init(struct dentry *dentry)
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
EXPORT_SYMBOL(ntl_br_entry_init);

void
ntl_br_entry_exit(void)
{
	ntl_debug("NTL Entry Exit");
}
EXPORT_SYMBOL(ntl_br_entry_exit);
