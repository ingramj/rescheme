#ifndef _RESCHEME_H
#define _RESCHEME_H

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


/**** object.c - object model and memory management. ****/

/* An rs_object can be any ReScheme data type. Right now there are only
   fixnums, but soon there will be others. */
typedef long rs_object;

/* Objects have a 2-bit tag that indicates their type. */
#define rs_object_tag_bits 2
#define rs_object_tag_mask 3

/* Fixnums */
typedef long rs_fixnum;

#define rs_fixnum_tag 1

#define rs_fixnum_min	  \
	(((((rs_fixnum)1) << (8 * sizeof(rs_object) - 1)) >> rs_object_tag_bits) + 1)

#define rs_fixnum_max (-(rs_fixnum_min))

static inline int rs_fixnum_p(rs_object obj) {
	return ((rs_fixnum)obj & rs_object_tag_mask) == rs_fixnum_tag;
}

static inline rs_object rs_fixnum_make(rs_fixnum val) {
	return (rs_object)(val << rs_object_tag_bits) + rs_fixnum_tag;
}

static inline rs_fixnum rs_fixnum_value(rs_object obj) {
	return (rs_fixnum)(obj >> rs_object_tag_bits);
}

/* Characters */

/* Assume that sizeof(short) >= 2. */
typedef short rs_character;

#define rs_character_tag 2

static inline int rs_character_p(rs_object obj) {
	return ((rs_character)obj & rs_object_tag_mask) == rs_character_tag;
}

static inline rs_object rs_character_make(rs_character val) {
	return (rs_object)(val << rs_object_tag_bits) + rs_character_tag;
}

static inline rs_character rs_character_value(rs_object obj) {
	return (rs_character)(obj >> rs_object_tag_bits);
}

/* Booleans, null, and end-of-file */

/* These objects all share a type tag of 3. However, since there are only 4
   "values", and they are (with the exception of the booleans) unrelated to one
   another, we'll just define them directly.
*/
#define rs_true  3  // 0011
#define rs_false 7  // 0111
#define rs_null  11 // 1011
#define rs_eof   15 // 1111

static inline int rs_boolean_p(rs_object obj) {
	return obj == rs_true || obj == rs_false;
}

static inline int rs_null_p(rs_object obj) {
	return obj == rs_null;
}

static inline int rs_eof_p(rs_object obj) {
	return obj == rs_eof;
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


/**** buffer.c - character buffers ****/

/* A growable, add-only character buffer. */
struct rs_buf;

/* Initialize a buffer. Trying to do anything to a buffer before calling this
   will probably cause an error.
*/
void rs_buf_init(struct rs_buf *buf);

/* Reset a buffer to an empty state. This does not free the structure itself,
   which can be reused. This should be called when you're done with the buffer,
   otherwise a memory leak could occur.
*/
void rs_buf_reset(struct rs_buf *buf);

/* Push a character into a buffer, and then return it. */
struct rs_buf *rs_buf_push(struct rs_buf *buf, char c);

/* Returns the string held in buf. Guaranteed to be NUL-terminated.
   NOTE: The string may be modified after it is returned. If you're going to
   keep it around for long, make a copy and use that instead.
 */
const char *rs_buf_str(struct rs_buf *buf);


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


/**** Private declarations. These should not be used directly. ****/

struct rs_buf {
	char *buf;
	size_t offset;
	size_t capacity;
};

void eprintf(int status, char const * const func, char const * const fmt, ...);

#endif
