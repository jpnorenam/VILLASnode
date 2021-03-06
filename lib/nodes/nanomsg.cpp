/** Node type: nanomsg
 *
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

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <cstring>

#include <villas/node.h>
#include <villas/nodes/nanomsg.hpp>
#include <villas/utils.hpp>
#include <villas/exceptions.hpp>

using namespace villas;
using namespace villas::node;
using namespace villas::utils;

int nanomsg_reverse(struct vnode *n)
{
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	if (vlist_length(&m->out.endpoints)  != 1 ||
	    vlist_length(&m->in.endpoints) != 1)
		return -1;

	char *subscriber = (char *) vlist_first(&m->in.endpoints);
	char *publisher = (char *) vlist_first(&m->out.endpoints);

	vlist_set(&m->in.endpoints, 0, publisher);
	vlist_set(&m->out.endpoints, 0, subscriber);

	return 0;
}

static int nanomsg_parse_endpoints(struct vlist *l, json_t *json)
{
	const char *ep;

	size_t i;
	json_t *json_val;

	switch (json_typeof(json)) {
		case JSON_ARRAY:
			json_array_foreach(json, i, json_val) {
				ep = json_string_value(json_val);
				if (!ep)
					return -1;

				vlist_push(l, strdup(ep));
			}
			break;

		case JSON_STRING:
			ep = json_string_value(json);

			vlist_push(l, strdup(ep));
			break;

		default:
			return -1;
	}

	return 0;
}

int nanomsg_parse(struct vnode *n, json_t *json)
{
	int ret;
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	json_error_t err;
	json_t *json_format = nullptr;
	json_t *json_out_endpoints = nullptr;
	json_t *json_in_endpoints = nullptr;

	ret = vlist_init(&m->out.endpoints);
	if (ret)
		return ret;

	ret = vlist_init(&m->in.endpoints);
	if (ret)
		return ret;

	ret = json_unpack_ex(json, &err, 0, "{ s?: o, s?: { s?: o }, s?: { s?: o } }",
		"format", &json_format,
		"out",
			"endpoints", &json_out_endpoints,
		"in",
			"endpoints", &json_in_endpoints
	);
	if (ret)
		throw ConfigError(json, err, "node-config-node-nanomsg");

	if (json_out_endpoints) {
		ret = nanomsg_parse_endpoints(&m->out.endpoints, json_out_endpoints);
		if (ret < 0)
			throw RuntimeError("Invalid type for 'publish' setting");
	}

	if (json_in_endpoints) {
		ret = nanomsg_parse_endpoints(&m->in.endpoints, json_in_endpoints);
		if (ret < 0)
			throw RuntimeError("Invalid type for 'subscribe' setting");
	}

	/* Format */
	m->formatter = json_format
			? FormatFactory::make(json_format)
			: FormatFactory::make("json");
	if (!m->formatter)
		throw ConfigError(json_format, "node-config-node-nanomsg-format", "Invalid format configuration");

	return 0;
}

char * nanomsg_print(struct vnode *n)
{
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	char *buf = nullptr;

	strcatf(&buf, "in.endpoints=[ ");

	for (size_t i = 0; i < vlist_length(&m->in.endpoints); i++) {
		char *ep = (char *) vlist_at(&m->in.endpoints, i);

		strcatf(&buf, "%s ", ep);
	}

	strcatf(&buf, "], out.endpoints=[ ");

	for (size_t i = 0; i < vlist_length(&m->out.endpoints); i++) {
		char *ep = (char *) vlist_at(&m->out.endpoints, i);

		strcatf(&buf, "%s ", ep);
	}

	strcatf(&buf, "]");

	return buf;
}

int nanomsg_start(struct vnode *n)
{
	int ret;
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	m->formatter->start(&n->in.signals, ~(int) SampleFlags::HAS_OFFSET);

	ret = m->in.socket = nn_socket(AF_SP, NN_SUB);
	if (ret < 0)
		throw RuntimeError("Failed to create nanomsg socket: {}", nn_strerror(errno));

	ret = m->out.socket = nn_socket(AF_SP, NN_PUB);
	if (ret < 0)
		throw RuntimeError("Failed to create nanomsg socket: {}", nn_strerror(errno));

	/* Subscribe to all topics */
	ret = nn_setsockopt(ret = m->in.socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
	if (ret < 0)
		return ret;

	/* Bind publisher to socket */
	for (size_t i = 0; i < vlist_length(&m->out.endpoints); i++) {
		char *ep = (char *) vlist_at(&m->out.endpoints, i);

		ret = nn_bind(m->out.socket, ep);
		if (ret < 0)
			throw RuntimeError("Failed to connect nanomsg socket to endpoint {}: {}", ep, nn_strerror(errno));
	}

	/* Connect subscribers socket */
	for (size_t i = 0; i < vlist_length(&m->in.endpoints); i++) {
		char *ep = (char *) vlist_at(&m->in.endpoints, i);

		ret = nn_connect(m->in.socket, ep);
		if (ret < 0)
			throw RuntimeError("Failed to connect nanomsg socket to endpoint {}: {}", ep, nn_strerror(errno));
	}

	return 0;
}

int nanomsg_stop(struct vnode *n)
{
	int ret;
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	ret = nn_close(m->in.socket);
	if (ret < 0)
		return ret;

	ret = nn_close(m->out.socket);
	if (ret < 0)
		return ret;

	delete m->formatter;

	return 0;
}

int nanomsg_type_stop()
{
	nn_term();

	return 0;
}

int nanomsg_read(struct vnode *n, struct sample * const smps[], unsigned cnt)
{
	struct nanomsg *m = (struct nanomsg *) n->_vd;
	int bytes;
	char data[NANOMSG_MAX_PACKET_LEN];

	/* Receive payload */
	bytes = nn_recv(m->in.socket, data, sizeof(data), 0);
	if (bytes < 0)
		return -1;

	return m->formatter->sscan(data, bytes, nullptr, smps, cnt);
}

int nanomsg_write(struct vnode *n, struct sample * const smps[], unsigned cnt)
{
	int ret;
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	size_t wbytes;

	char data[NANOMSG_MAX_PACKET_LEN];

	ret = m->formatter->sprint(data, sizeof(data), &wbytes, smps, cnt);
	if (ret <= 0)
		return -1;

	ret = nn_send(m->out.socket, data, wbytes, 0);
	if (ret < 0)
		return ret;

	return cnt;
}

int nanomsg_poll_fds(struct vnode *n, int fds[])
{
	int ret;
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	int fd;
	size_t len = sizeof(fd);

	ret = nn_getsockopt(m->in.socket, NN_SOL_SOCKET, NN_RCVFD, &fd, &len);
	if (ret)
		return ret;

	fds[0] = fd;

	return 1;
}

int nanomsg_netem_fds(struct vnode *n, int fds[])
{
	struct nanomsg *m = (struct nanomsg *) n->_vd;

	fds[0] = m->out.socket;

	return 1;
}

static struct vnode_type p;

__attribute__((constructor(110)))
static void register_plugin() {
	p.name		= "nanomsg";
	p.description	= "scalability protocols library (libnanomsg)";
	p.vectorize	= 0;
	p.size		= sizeof(struct nanomsg);
	p.type.stop	= nanomsg_type_stop;
	p.parse		= nanomsg_parse;
	p.print		= nanomsg_print;
	p.start		= nanomsg_start;
	p.stop		= nanomsg_stop;
	p.read		= nanomsg_read;
	p.write		= nanomsg_write;
	p.reverse	= nanomsg_reverse;
	p.poll_fds	= nanomsg_poll_fds;
	p.netem_fds	= nanomsg_netem_fds;

	if (!node_types)
		node_types = new NodeTypeList();

	node_types->push_back(&p);
}
