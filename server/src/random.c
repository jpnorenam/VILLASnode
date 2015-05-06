/** Generate random packages on stdout.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2015, Institute for Automation of Complex Power Systems, EONERC
 * @file
 * @addtogroup tools Test and debug tools
 * @{
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>

#include "config.h"
#include "utils.h"
#include "msg.h"

#define CLOCKID	CLOCK_REALTIME

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Usage: %s VALUES RATE\n", argv[0]);
		printf("  VALUES is the number of values a message contains\n");
		printf("  RATE   how many messages per second\n\n");

		printf("Simulator2Simulator Server %s (built on %s %s)\n",
			BLU(VERSION), MAG(__DATE__), MAG(__TIME__));
		printf(" Copyright 2015, Institute for Automation of Complex Power Systems, EONERC\n");
		printf(" Steffen Vogel <StVogel@eonerc.rwth-aachen.de>\n");

		exit(EXIT_FAILURE);
	}

	uint64_t runs;
	int rate = atoi(argv[2]);
	struct msg m = MSG_INIT(atoi(argv[1]));

	/* Setup timer */
	struct itimerspec its = {
		.it_interval = timespec_rate(rate),
		.it_value = { 1, 0 }
	};

	int tfd = timerfd_create(CLOCK_REALTIME, 0);
	if (tfd < 0)
		serror("Failed to create timer");

	if (timerfd_settime(tfd, 0, &its, NULL))
		serror("Failed to start timer");

	/* Print header */
	fprintf(stderr, "# %-6s%-12s\n", "seq", "data");

	while (1) {
		/* Block until 1/p->rate seconds elapsed */
		read(tfd, &runs, sizeof(runs));
		
		msg_random(&m);
		msg_fprint(stdout, &m);
		
		fflush(stdout);

		m.sequence++;
	}

	close(tfd);

	return 0;
}

/** @} */