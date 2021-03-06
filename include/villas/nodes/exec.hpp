/** Node-type for exec node-types.
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
 * @ingroup node
 * @addtogroup exec Execute node-type as a sub-process
 * @{
 */

#pragma once

#include <villas/popen.hpp>
#include <villas/format.hpp>

/* Forward declarations */
struct vnode;
struct sample;

/** Node-type for signal generation.
 * @see node_type
 */
struct exec {
	std::unique_ptr<villas::utils::Popen> proc;

	bool flush;
	bool shell;
	std::string working_dir;
	std::string command;
	villas::utils::Popen::arg_list arguments;
	villas::utils::Popen::env_map environment;

	villas::node::Format *formatter;
};

/** @see node_type::print */
char * exec_print(struct vnode *n);

/** @see node_type::parse */
int exec_parse(struct vnode *n, json_t *json);

/** @see node_type::start */
int exec_open(struct vnode *n);

/** @see node_type::stop */
int exec_close(struct vnode *n);

/** @see node_type::read */
int exec_read(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @see node_type::write */
int exec_write(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @} */
