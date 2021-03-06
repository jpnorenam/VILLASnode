/** Main routine.
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

#include <cstdlib>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <exception>
#include <atomic>

#include <villas/tool.hpp>
#include <villas/node/config.h>
#include <villas/version.hpp>
#include <villas/utils.hpp>
#include <villas/super_node.hpp>
#include <villas/plugin.hpp>
#include <villas/api/request.hpp>
#include <villas/memory.h>
#include <villas/node.h>
#include <villas/path.h>
#include <villas/api.hpp>
#include <villas/hook.hpp>
#include <villas/format.hpp>
#include <villas/colors.hpp>
#include <villas/web.hpp>
#include <villas/log.hpp>
#include <villas/exceptions.hpp>
#include <villas/kernel/kernel.hpp>
#include <villas/kernel/rt.hpp>

#ifdef WITH_NODE_OPAL
  #include <villas/nodes/opal.hpp>
#endif

using namespace villas;
using namespace villas::node;
using namespace villas::plugin;

namespace villas {
namespace node {
namespace tools {

class Node : public Tool {

public:
	Node(int argc, char *argv[]) :
		Tool(argc, argv, "node")
	{ }

protected:
	SuperNode sn;

	std::string uri;

	void handler(int signal, siginfo_t *sinfo, void *ctx)
	{
		switch (signal)  {
			case  SIGALRM:
				logger->info("Reached timeout. Terminating...");
				break;

			default:
				logger->info("Received {} signal. Terminating...", strsignal(signal));
		}

		sn.setState(State::STOPPING);
	}

	void usage()
	{
		std::cout << "Usage: villas-node [OPTIONS] [CONFIG]" << std::endl
			<< "  OPTIONS is one or more of the following options:" << std::endl
			<< "    -h      show this usage information" << std::endl
			<< "    -d LVL  set logging level" << std::endl
			<< "    -V      show the version of the tool" << std::endl << std::endl
			<< "  CONFIG is the path to an optional configuration file" << std::endl
			<< "         if omitted, VILLASnode will start without a configuration" << std::endl
			<< "         and wait for provisioning over the web interface." << std::endl << std::endl
#ifdef WITH_NODE_OPAL
			<< "Usage: villas-node OPAL_ASYNC_SHMEM_NAME OPAL_ASYNC_SHMEM_SIZE OPAL_PRINT_SHMEM_NAME" << std::endl
			<< "  This type of invocation is used by OPAL-RT Asynchronous processes." << std::endl
			<< "  See in the RT-LAB User Guide for more information." << std::endl << std::endl
#endif /* WITH_NODE_OPAL */

		<< "Supported node-types:" << std::endl;
		for (auto *vt : *node_types)
			std::cout << " - " << std::left << std::setw(18) << node_type_name(vt) << vt->description << std::endl;
		std::cout << std::endl;

		std::cout << "Supported IO formats:" << std::endl;
		for (Plugin *p : Registry::lookup<FormatFactory>())
			std::cout << " - " << std::left << std::setw(18) << p->getName() << p->getDescription() << std::endl;
		std::cout << std::endl;

#ifdef WITH_HOOKS
		std::cout << "Supported hooks:" << std::endl;
		for (Plugin *p : Registry::lookup<HookFactory>())
			std::cout << " - " << std::left << std::setw(18) << p->getName() << p->getDescription() << std::endl;
		std::cout << std::endl;
#endif /* WITH_HOOKS */

#ifdef WITH_API
		std::cout << "Supported API commands:" << std::endl;
		for (Plugin *p : Registry::lookup<api::RequestFactory>())
			std::cout << " - " << std::left << std::setw(18) << p->getName() << p->getDescription() << std::endl;
		std::cout << std::endl;
#endif /* WITH_API */

		printCopyright();
	}

	void parse()
	{
		/* Check arguments */
#ifdef WITH_NODE_OPAL
		if (argc != 4) {
			usage();
			exit(EXIT_FAILURE);
		}

		opal_register_region(argc, argv);

		uri = "villas-node.conf";
#else

		/* Parse optional command line arguments */
		int c;
		while ((c = getopt(argc, argv, "hVd:")) != -1) {
			switch (c) {
				case 'V':
					printVersion();
					exit(EXIT_SUCCESS);

				case 'd':
					logging.setLevel(optarg);
					break;

				case 'h':
				case '?':
					usage();
					exit(c == '?' ? EXIT_FAILURE : EXIT_SUCCESS);
			}

			continue;
		}

		if (argc == optind + 1)
			uri = argv[optind];
		else if (argc != optind) {
			usage();
			exit(EXIT_FAILURE);
		}
#endif /* ENABLE_OPAL_ASYNC */
	}

	int main()
	{
		if (!uri.empty())
			sn.parse(uri);
		else
			logger->warn("No configuration file specified. Starting unconfigured. Use the API to configure this instance.");

		sn.check();
		sn.prepare();
		sn.start();
		sn.run();
		sn.stop();

		return 0;
	}
};

} /* namespace tools */
} /* namespace node */
} /* namespace villas */

int main(int argc, char *argv[])
{
	villas::node::tools::Node t(argc, argv);

	return t.run();
}

/** @} */

