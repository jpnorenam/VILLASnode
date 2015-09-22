/** Interface related functions.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2015, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of S2SS. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#include <netlink/route/link.h>
#include <netlink/route/route.h>

#include "if.h"
#include "tc.h"
#include "nl.h"
#include "socket.h"
#include "utils.h"
#include "checks.h"

/** Linked list of interfaces. */
struct list interfaces;

struct interface * if_create(struct rtnl_link *link)
{
	struct interface *i = alloc(sizeof(struct interface));
	
	i->nl_link = link;

	debug(3, "Created interface '%s'", rtnl_link_get_name(i->nl_link));

	if_get_irqs(i);

	list_init(&i->sockets, NULL);
	list_push(&interfaces, i);

	return i;
}

void if_destroy(struct interface *i)
{
	/* List members are freed by the nodes they belong to. */
	list_destroy(&i->sockets);
	
	rtnl_qdisc_put(i->tc_qdisc);

	free(i);
}

int if_start(struct interface *i, int affinity)
{
	info("Starting interface '%s' which is used by %u sockets", rtnl_link_get_name(i->nl_link), list_length(&i->sockets));

	{ INDENT
		/* Set affinity for network interfaces (skip _loopback_ dev) */
		if_set_affinity(i, affinity);
		
		/* Assign fwmark's to socket nodes which have netem options */
		int ret, mark = 0;
		FOREACH(&i->sockets, it) {
			struct socket *s = it->socket;
			if (s->tc_qdisc)
				s->mark = 1 + mark++;
		}

		/* Abort if no node is using netem */
		if (mark == 0)
			return 0;

		/* Check if all kernel modules are loaded */
		if (check_kernel_module("sch_prio"))
			error("Missing kernel module: sch_prio");
		if (check_kernel_module("sch_netem"))
			error("Missing kernel module: sch_netem");		
		if (check_kernel_module("cls_fw"))
			error("Missing kernel module: cls_fw");

		/* Replace root qdisc */
		ret = tc_prio(i, &i->tc_qdisc, TC_HANDLE(1, 0), TC_H_ROOT, mark);
		if (ret)
			error("Failed to setup priority queuing discipline: %s", nl_geterror(ret));

		/* Create netem qdisks and appropriate filter per netem node */
		FOREACH(&i->sockets, it) {
			struct socket *s = it->socket;
			if (s->tc_qdisc) {
				ret = tc_mark(i,  &s->tc_classifier, TC_HANDLE(1, s->mark), s->mark);
				if (ret)
					error("Failed to setup FW mark classifier: %s", nl_geterror(ret));
				
				char *buf = tc_print(s->tc_qdisc);
				debug(5, "Starting network emulation on interface '%s' for FW mark %u: %s",
					rtnl_link_get_name(i->nl_link), s->mark, buf);
				free(buf);

				ret = tc_netem(i, &s->tc_qdisc, TC_HANDLE(0x1000+s->mark, 0), TC_HANDLE(1, s->mark));
				if (ret)
					error("Failed to setup netem qdisc: %s", nl_geterror(ret));
			}
		}
	}

	return 0;
}

int if_stop(struct interface *i)
{
	info("Stopping interface '%s'", rtnl_link_get_name(i->nl_link));

	{ INDENT
		if_set_affinity(i, -1L);

		if (i->tc_qdisc)
			tc_reset(i);
	}

	return 0;
}

int if_get_egress(struct sockaddr *sa, struct rtnl_link **link)
{
	int ifindex = -1;

	switch (sa->sa_family) {
		case AF_INET:
		case AF_INET6: {
			struct sockaddr_in *sin = (struct sockaddr_in *) sa;
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
			
			struct nl_addr *addr = (sa->sa_family == AF_INET)
				? nl_addr_build(sin->sin_family, &sin->sin_addr.s_addr, sizeof(sin->sin_addr.s_addr))
				: nl_addr_build(sin6->sin6_family, sin6->sin6_addr.s6_addr, sizeof(sin6->sin6_addr));
			
			ifindex = nl_get_egress(addr);
			if (ifindex < 0)
				error("Netlink error: %s", nl_geterror(ifindex));
			break;
		}
		
		case AF_PACKET: {
			struct sockaddr_ll *sll = (struct sockaddr_ll *) sa;
			
			ifindex = sll->sll_ifindex;
			break;
		}
	}
	
	struct nl_cache *cache = nl_cache_mngt_require("route/link");
	*link = rtnl_link_get(cache, ifindex);
	if (!*link)
		return -1;
	
	return 0;
}

int if_get_irqs(struct interface *i)
{
	char dirname[NAME_MAX];
	int irq, n = 0;

	snprintf(dirname, sizeof(dirname), "/sys/class/net/%s/device/msi_irqs/", rtnl_link_get_name(i->nl_link));
	DIR *dir = opendir(dirname);
	if (dir) {
		memset(&i->irqs, 0, sizeof(char) * IF_IRQ_MAX);

		struct dirent *entry;
		while ((entry = readdir(dir)) && n < IF_IRQ_MAX) {
			irq = atoi(entry->d_name);
			if (irq)
				i->irqs[n++] = irq;
		}

		closedir(dir);
	}
	
	debug(6, "Found %u IRQs for interface '%s'", n, rtnl_link_get_name(i->nl_link));

	return 0;
}

int if_set_affinity(struct interface *i, int affinity)
{
	char filename[NAME_MAX];
	FILE *file;

	for (int n = 0; n < IF_IRQ_MAX && i->irqs[n]; n++) {
		snprintf(filename, sizeof(filename), "/proc/irq/%u/smp_affinity", i->irqs[n]);

		file = fopen(filename, "w");
		if (file) {
			if (fprintf(file, "%8x", affinity) < 0)
				error("Failed to set affinity for IRQ %u", i->irqs[n]);

			fclose(file);
			debug(5, "Set affinity of IRQ %u for interface '%s' to %#x", i->irqs[n], rtnl_link_get_name(i->nl_link), affinity);
		}
		else
			error("Failed to set affinity for interface '%s'", rtnl_link_get_name(i->nl_link));
	}

	return 0;
}

struct interface * if_lookup_index(int index)
{
	FOREACH(&interfaces, it) {
		if (rtnl_link_get_ifindex(it->interface->nl_link) == index)
			return it->interface;
	}

	return NULL;
}

