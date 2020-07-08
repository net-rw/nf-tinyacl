/* ntl-nf-bridge.c
 *
 * Copyright (c) 2020, Leesoo Ahn <yisooan@fedoraproject.org>
 *
 * This software is distributed under the terms of the BSD or GPL license.
 */

#include <linux/if_ether.h>
#include <linux/netfilter_bridge.h>

#include "ntl-priv.h"
#include "ntl-entry.h"
#include "ntl-nf-bridge.h"

static unsigned int ntl_br_pre_routing_hook(void *priv,
			struct sk_buff *skb,
			const struct nf_hook_state *nhs)
{
	struct net_device *from = nhs->in;
	struct acl_entry *entry;
	struct ethhdr *eth;
	u32 flag;

	eth = eth_hdr(skb);
	if (!eth) {
		ntl_debug("Not eth frame, just go to upper layer");
		return NF_ACCEPT;
	}

	rcu_read_lock();
	entry = ntl_entry_lookup_by_mac(eth->h_source);
	if (entry)
		flag = NTL_ENTRY_DENY_FG;
	else
		flag = NTL_ENTRY_ALLOW_FG;
	rcu_read_unlock();

	switch (flag) {
	case NTL_ENTRY_DENY_FG:
		return NF_DROP;
	default:
		return NF_ACCEPT;
	}
}

static struct nf_hook_ops ntl_nf_br_hooks[] = {
	{
		.hook      = ntl_br_pre_routing_hook,
		.pf        = PF_BRIDGE,
		.hooknum   = NF_BR_PRE_ROUTING,
		.priority  = NF_BR_PRI_FIRST,
	},
};

int
ntl_nf_bridge_init(void *parent_dentry)
{
	int ret = -1;

	ntl_debug("NTL NF Bridge Init");

	ret = nf_register_hooks(ntl_nf_br_hooks,
			ARRAY_SIZE(ntl_nf_br_hooks));
	if (ret < 0) {
		ntl_debug("Failed to register netfilter hooks");
	}

	return ret;
}
EXPORT_SYMBOL(ntl_nf_bridge_init);

void
ntl_nf_bridge_exit(void)
{
	ntl_debug("NTL NF Bridge Exit");

	nf_unregister_hooks(ntl_nf_br_hooks,
			ARRAY_SIZE(ntl_nf_br_hooks));
}
EXPORT_SYMBOL(ntl_nf_bridge_exit);
