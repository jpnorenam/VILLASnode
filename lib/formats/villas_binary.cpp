/** Message related functions.
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

#include <cstring>
#include <arpa/inet.h>

#include <villas/formats/villas_binary.hpp>
#include <villas/formats/msg.hpp>
#include <villas/formats/msg_format.hpp>
#include <villas/sample.h>
#include <villas/utils.hpp>

using namespace villas::node;

int VillasBinaryFormat::sprint(char *buf, size_t len, size_t *wbytes, const struct sample * const smps[], unsigned cnt)
{
	int ret;
	unsigned i = 0;
	char *ptr = buf;

	for (i = 0; i < cnt; i++) {
		struct msg *msg = (struct msg *) ptr;
		const struct sample *smp = smps[i];

		if (ptr + MSG_LEN(smp->length) > buf + len)
			break;

		ret = msg_from_sample(msg, smp, smp->signals);
		if (ret)
			return ret;

		if (web) {
			/** @todo convert to little endian */
		}
		else
			msg_hton(msg);

		ptr += MSG_LEN(smp->length);
	}

	if (wbytes)
		*wbytes = ptr - buf;

	return i;
}

int VillasBinaryFormat::sscan(const char *buf, size_t len, size_t *rbytes, struct sample * const smps[], unsigned cnt)
{
	int ret, values;
	unsigned i = 0;
	const char *ptr = buf;

	if (len % 4 != 0)
		return -1; /* Packet size is invalid: Must be multiple of 4 bytes */

	for (i = 0; i < cnt; i++) {
		struct msg *msg = (struct msg *) ptr;
		struct sample *smp = smps[i];

		smp->signals = signals;

		/* Complete buffer has been parsed */
		if (ptr == buf + len)
			break;

		/* Check if header is still in buffer bounaries */
		if (ptr + sizeof(struct msg) > buf + len)
			return -2; /* Invalid msg received */

		values = web ? msg->length : ntohs(msg->length);

		/* Check if remainder of message is in buffer boundaries */
		if (ptr + MSG_LEN(values) > buf + len)
			return -3; /*Invalid msg receive */

		if (web) {
			/** @todo convert from little endian */
		}
		else
			msg_ntoh(msg);

		ret = msg_to_sample(msg, smp, signals);
		if (ret)
			return ret; /* Invalid msg received */

		ptr += MSG_LEN(smp->length);
	}

	if (rbytes)
		*rbytes = ptr - buf;

	return i;
}

static VillasBinaryFormatPlugin<false> p1;
static VillasBinaryFormatPlugin<true> p2;
