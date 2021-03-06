/** Node type: kafka
 *
 * @file
 * @author Juan Pablo Noreña <jpnorenam@unal.edu.co>
 * @copyright 2021, Universidad Nacional de Colombia
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
 * @addtogroup kafka kafka node type
 * @ingroup node
 * @{
 */

#pragma once

#include <librdkafka/rdkafka.h>

#include <villas/pool.h>
#include <villas/format.hpp>
#include <villas/queue_signalled.h>

/* Forward declarations */
struct vnode;

struct kafka {
	struct queue_signalled queue;
	struct pool pool;

	double timeout;			/**< Timeout in seconds. */
	char *server;			/**< Hostname/IP:Port address of the bootstrap server. */
	char *protocol;			/**< Security protocol. */
	char *produce;			/**< Producer topic. */
	char *consume;			/**< Consumer topic. */
	char *client_id;		/**< Client ID. */

	struct {
		rd_kafka_t *client;
		rd_kafka_topic_t *topic;
	} producer;

	struct {
		rd_kafka_t *client;
		char *group_id;		/**< Group id. */
	} consumer;

	struct {
		char *ca;	/**< SSL CA file. */
	} ssl;

	struct {
		char *mechanism;	/**< SASL mechanism. */
		char *username;		/**< SSL CA path. */
		char *password;		/**< SSL certificate. */
	} sasl;

	villas::node::Format *formatter;
};

/** @see node_type::reverse */
int kafka_reverse(struct vnode *n);

/** @see node_type::print */
char * kafka_print(struct vnode *n);

/** @see node_type::prepare */
int kafka_prepare(struct vnode *n);

/** @see node_type::parse */
int kafka_parse(struct vnode *n, json_t *json);

/** @see node_type::start */
int kafka_start(struct vnode *n);

/** @see node_type::destroy */
int kafka_destroy(struct vnode *n);

/** @see node_type::stop */
int kafka_stop(struct vnode *n);

/** @see node_type::type_start */
int kafka_type_start(villas::node::SuperNode *sn);

/** @see node_type::type_stop */
int kafka_type_stop();

/** @see node_type::read */
int kafka_read(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @see node_type::write */
int kafka_write(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @} */
