/** Node-type for signal generation.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
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
 * @ingroup node
 * @addtogroup signal Signal generation node-type-
 * @{
 */

#pragma once

#include "timing.h"
#include "task.h"

/* Forward declarations */
struct node;
struct sample;

enum signal_type {
	SIGNAL_TYPE_RANDOM,
	SIGNAL_TYPE_SINE,
	SIGNAL_TYPE_SQUARE,
	SIGNAL_TYPE_TRIANGLE,
	SIGNAL_TYPE_RAMP,
	SIGNAL_TYPE_COUNTER,
	SIGNAL_TYPE_CONSTANT,
	SIGNAL_TYPE_MIXED
};

/** Node-type for signal generation.
 * @see node_type
 */
struct signal {
	struct task task;		/**< Timer for periodic events. */
	int rt;				/**< Real-time mode? */

	enum signal_type type;		/**< Signal type */

	double rate;			/**< Sampling rate. */
	double frequency;		/**< Frequency of the generated signals. */
	double amplitude;		/**< Amplitude of the generated signals. */
	double stddev;			/**< Standard deviation of random signals (normal distributed). */
	double offset;			/**< A constant bias. */
	
	double *last;			/**< The values from the previous period which are required for random walk. */

	int values;			/**< The number of values which will be emitted by this node. */
	int limit;			/**< The number of values which should be generated by this node. <0 for infinitve. */

	struct timespec started;	/**< Point in time when this node was started. */
	int counter;			/**< The number of packets already emitted. */
};

/** @see node_type::print */
char * signal_print(struct node *n);

/** @see node_type::parse */
int signal_parse(struct node *n, json_t *cfg);

/** @see node_type::open */
int signal_open(struct node *n);

/** @see node_type::close */
int signal_close(struct node *n);

/** @see node_type::read */
int signal_read(struct node *n, struct sample *smps[], unsigned cnt);

enum signal_type signal_lookup_type(const char *type);

void signal_get(struct signal *s, struct sample *t, struct timespec *now);

/** @} */
