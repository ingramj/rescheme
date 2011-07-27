#include "rescheme.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024


void eprintf(int status, char const * const func, char const * const fmt, ...)
{
	int olderr = errno;
	char buf[BUFSIZE];
	int bytes;
	va_list argv;

	(void) fflush(stderr);

	bytes = snprintf(buf, BUFSIZE, "%s: ", func);

	va_start(argv, fmt);
	bytes += vsnprintf(buf + bytes, BUFSIZE - bytes, fmt, argv);
	va_end(argv);

	if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') {
		bytes += snprintf(buf + bytes, BUFSIZE - bytes, " %s",
		                  strerror(status));
	}

	fprintf(stderr, "%s\n", buf);
	if (bytes >= BUFSIZE - 1) {
		fprintf(stderr, "warning: error message from %s has been truncated.\n", func);
	}

	errno = olderr;
}
