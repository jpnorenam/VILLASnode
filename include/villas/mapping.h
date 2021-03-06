/** Sample value remapping for mux.
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

#pragma once

#include <jansson.h>

#include <villas/stats.hpp>
#include <villas/common.hpp>
#include <villas/node_list.hpp>

#define RE_MAPPING_INDEX "[a-zA-Z0-9_]+"
#define RE_MAPPING_RANGE "(" RE_MAPPING_INDEX ")(?:-(" RE_MAPPING_INDEX "))?"

#define RE_MAPPING_STATS "stats\\.([a-z]+)\\.([a-z]+)"
#define RE_MAPPING_HDR   "hdr\\.(sequence|length)"
#define RE_MAPPING_TS    "ts\\.(origin|received)"
#define RE_MAPPING_DATA1 "data\\[" RE_MAPPING_RANGE "\\]"
#define RE_MAPPING_DATA2 "(?:data\\.)?(" RE_MAPPING_INDEX ")"
#define RE_MAPPING       "(?:(" RE_NODE_NAME ")\\.(?:" RE_MAPPING_STATS "|" RE_MAPPING_HDR "|" RE_MAPPING_TS "|" RE_MAPPING_DATA1 "|" RE_MAPPING_DATA2 ")|(" RE_NODE_NAME ")(?:\\[" RE_MAPPING_RANGE "\\])?)"

/* Forward declarations */
struct vnode;
struct sample;
struct signal;
struct vlist;

enum class MappingType {
	UNKNOWN,
	DATA,
	STATS,
	HEADER,
	TIMESTAMP
};

enum class MappingHeaderType {
	LENGTH,
	SEQUENCE
};

enum class MappingTimestampType {
	ORIGIN,
	RECEIVED
};

struct mapping_entry {
	char *node_name;
	struct vnode *node;		/**< The node to which this mapping refers. */

	enum MappingType type;		/**< The mapping type. Selects one of the union fields below. */

	/** The number of values which is covered by this mapping entry.
	 *
	 * A value of 0 indicates that all remaining values starting from the offset of a sample should be mapped.
	 */
	int length;
	unsigned offset;		/**< Offset of this mapping entry within sample::data */

	union {
		struct {
			int offset;
			struct signal *signal;

			char *first;
			char *last;
		} data;

		struct {
			enum villas::Stats::Metric metric;
			enum villas::Stats::Type type;
		} stats;

		struct {
			enum MappingHeaderType type;
		} header;

		struct {
			enum MappingTimestampType type;
		} timestamp;
	};
};

int mapping_entry_prepare(struct mapping_entry *me, villas::node::NodeList &nodes);

int mapping_entry_update(const struct mapping_entry *me, struct sample *remapped, const struct sample *original);

int mapping_entry_init(struct mapping_entry *me);

int mapping_entry_destroy(struct mapping_entry *me);

int mapping_entry_parse(struct mapping_entry *me, json_t *json);

int mapping_entry_parse_str(struct mapping_entry *e, const std::string &str);

int mapping_entry_to_str(const struct mapping_entry *me, unsigned index, char **str);

int mapping_list_parse(struct vlist *ml, json_t *json);

int mapping_list_prepare(struct vlist *ml, villas::node::NodeList &nodes);

int mapping_list_remap(const struct vlist *ml, struct sample *remapped, const struct sample *original);
