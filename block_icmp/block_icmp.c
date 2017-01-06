/*
	A netfilter module to block ICMP packets
	Author: Angelo Poerio <angelo.poerio@gmail.com>
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");              
MODULE_AUTHOR("Angelo Poerio");      
MODULE_DESCRIPTION("A netfilter module to block ICMP packets");  
MODULE_VERSION("0.1");

static struct nf_hook_ops nfho;
struct sk_buff *skb_local;
struct iphdr *ip_header;

static unsigned int block_func(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *)) {

        skb_local = skb;

	if(!skb_local) { /* sanity check, avoid null pointers! */
		return NF_ACCEPT;
	}
	
	ip_header = (struct iphdr *)skb_network_header(skb_local); /* get IP HEADER */

	if(!ip_header) /* sanity check */
		return NF_ACCEPT;

	if(ip_header->protocol == IPPROTO_ICMP)
	{
		printk(KERN_INFO "Dropping the ICMP packet!\n");
		return NF_DROP; /* drop the ICMP packet! */
	}
	   
	return NF_ACCEPT; /* everything else gets accepted */

}

static int __init initialize(void) {
        nfho.hook     = block_func; 
        nfho.hooknum  = NF_INET_PRE_ROUTING; 
        nfho.pf       = PF_INET; 
        nfho.priority = NF_IP_PRI_FIRST; /* highest priority in the CHAIN */
        nf_register_hook(&nfho); 
        return 0;
}

static void __exit cleanup(void) {
        nf_unregister_hook(&nfho); 
}

module_init(initialize);
module_exit(cleanup);
