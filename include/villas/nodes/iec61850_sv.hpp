/** Node type: IEC 61850-9-2 (Sampled Values)
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

/**
 * @addtogroup iec61850_sv IEC 61850-9-2 (Sampled Values) node type
 * @ingroup node
 * @{
 */

#pragma once

#include <cstdint>

#include <libiec61850/sv_publisher.h>
#include <libiec61850/sv_subscriber.h>

#include <villas/queue_signalled.h>
#include <villas/pool.h>
#include <villas/list.h>
#include <villas/nodes/iec61850.hpp>

/* Forward declarations */
struct vnode;

struct iec61850_sv {
	char *interface;
	int app_id;
	struct ether_addr dst_address;

	struct {
		bool enabled;

		SVSubscriber subscriber;
		SVReceiver receiver;

		struct queue_signalled queue;
		struct pool pool;

		struct vlist signals;		/**< Mappings of type struct iec61850_type_descriptor */
		int total_size;
	} in;

	struct {
		bool enabled;

		SVPublisher publisher;
		SVPublisher_ASDU asdu;

		char *svid;

		int vlan_priority;
		int vlan_id;
		int smpmod;
		int smprate;
		int confrev;

		struct vlist signals;		/**< Mappings of type struct iec61850_type_descriptor */
		int total_size;
	} out;
};

/** @see node_type::type_stop */
int iec61850_sv_type_stop();

/** @see node_type::parse */
int iec61850_sv_parse(struct vnode *n, json_t *json);

/** @see node_type::print */
char * iec61850_sv_print(struct vnode *n);

/** @see node_type::start */
int iec61850_sv_start(struct vnode *n);

/** @see node_type::stop */
int iec61850_sv_stop(struct vnode *n);

/** @see node_type::destroy */
int iec61850_sv_destroy(struct vnode *n);

/** @see node_type::read */
int iec61850_sv_read(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @see node_type::write */
int iec61850_sv_write(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @see node_type::fd */
int iec61850_sv_fd(struct vnode *n);

/** @} */
