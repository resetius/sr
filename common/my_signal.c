/*$Id$*/
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "my_signal.h"

void set_signal(int signo, sig_handler_t hndl)
{
	struct sigaction sa;
	sa.sa_handler = hndl;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, signo);
	sa.sa_flags = 0;
	
	if (sigaction(signo, &sa, NULL) < 0) {
		fprintf(stderr, "can't catch signal %d: %s\n", 
				signo, strerror(errno));
		exit(1);
	}
}


