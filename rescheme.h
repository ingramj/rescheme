#ifndef _RESCHEME_H
#define _RESCHEME_H

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


/**** error.c - error-reporting. ****/

/* The error macros have a printf-like interface. If the format string ends with
   a ':', then the result of strerror(3) is appened. The "_s" varieties take an
   integer status code to pass to strerror(3), and the normal varieties use
   errno.
*/

/* Display an error message and exit. */
#define rs_fatal(...) do { \
		eprintf(errno, __func__, __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0)
#define rs_fatal_s(status, ...) do { \
		eprintf(status, __func__, __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0)

/* Display an error message. */
#define rs_nonfatal(...) eprintf(errno, __func__, __VA_ARGS__)
#define rs_nonfatal_s(status, ...) eprintf(status, __func__, __VA_ARGS__);


/* Debugging messages. */
#ifndef DEBUG
# define TRACE(...)
#else
# define TRACE(...) rs_nonfatal(__VA_ARGS__)
#endif

/* This function is used by the preceding macros, and should not be called
   directly. */
void eprintf(int status, char const * const func, char const * const fmt, ...);


/**** object.c - object model and memory management. ****/

/* Eventually, rs_object will be able to hold every object type. For now, our
   only objects are fixnums. */
typedef long rs_object;

/* An integer that is restricted to a fixed range. */
typedef long rs_fixnum;

/* The minimum and maximum values for fixnums will change as the implementation
   evolves. Specifically, a few bits will be lost to "type tags". */
#define rs_fixnum_min LONG_MIN;
#define rs_fixnum_max LONG_MAX;

/* Convert a fixnum into an object. */
static inline rs_object rs_fixnum_make(rs_fixnum val) {
	return (rs_object) val;
}

/* Convert an object into a fixnum. */
static inline rs_fixnum rs_fixnum_value(rs_object obj) {
	return (rs_fixnum) obj;
}


/**** read.c - s-expression parsing. ****/

/* Read an s-expression from a file, and return the resulting object. */
rs_object rs_read(FILE *in);


/**** eval.c - object evaluation. ****/

/* Evaluate expr, and return the result. */
rs_object rs_eval(rs_object expr);


/**** write.c - s-expression output. ****/

/* Write obj's corresponding s-expression to a file, and return the
   number of bytes written */
int rs_write(FILE *out, rs_object obj);


#endif
