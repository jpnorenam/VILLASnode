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
 * @addtogroup signal_generator Signal generation node-type-
 * @{
 */

#pragma once

#include <villas/timing.h>
#include <villas/task.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct node;
struct sample;

enum signal_generator_type {
	SIGNAL_GENERATOR_TYPE_RANDOM,
	SIGNAL_GENERATOR_TYPE_SINE,
	SIGNAL_GENERATOR_TYPE_SQUARE,
	SIGNAL_GENERATOR_TYPE_TRIANGLE,
	SIGNAL_GENERATOR_TYPE_RAMP,
	SIGNAL_GENERATOR_TYPE_COUNTER,
	SIGNAL_GENERATOR_TYPE_CONSTANT,
	SIGNAL_GENERATOR_TYPE_MIXED
};

/** Node-type for signal generation.
 * @see node_type
 */
struct signal_generator {
	struct task task;		/**< Timer for periodic events. */
	int rt;				/**< Real-time mode? */

	enum signal_generator_type type; /**< Signal type */

	double rate;			/**< Sampling rate. */
	double frequency;		/**< Frequency of the generated signals. */
	double amplitude;		/**< Amplitude of the generated signals. */
	double stddev;			/**< Standard deviation of random signals (normal distributed). */
	double offset;			/**< A constant bias. */
	int monitor_missed;		/**< Boolean, if set, node counts missed steps and warns user. */

	double *last;			/**< The values from the previous period which are required for random walk. */

	int values;			/**< The number of values which will be emitted by this node. */
	int limit;			/**< The number of values which should be generated by this node. <0 for infinitve. */

	struct timespec started;	/**< Point in time when this node was started. */
	int counter;			/**< The number of packets already emitted. */
	int missed_steps;		/**< Total number of missed steps. */
};

/** @see node_type::print */
char * signal_generator_print(struct node *n);

/** @see node_type::parse */
int signal_generator_parse(struct node *n, json_t *cfg);

/** @see node_type::start */
int signal_generator_start(struct node *n);

/** @see node_type::stop */
int signal_generator_stop(struct node *n);

/** @see node_type::read */
int signal_generator_read(struct node *n, struct sample *smps[], unsigned cnt, unsigned *release);

enum signal_generator_type signal_generator_lookup_type(const char *type);

/** @} */

#ifdef __cplusplus
}
#endif
