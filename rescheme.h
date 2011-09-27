#ifndef _RESCHEME_H
#define _RESCHEME_H

#define _POSIX_C_SOURCE 200809L

/* The ReScheme "public" API. Typedefs, functions, an macros declared here may
   be directly used in any compilation unit.
*/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>



/**** object.c - object model. ****/

/* An rs_object can be any ReScheme data type. Right now there are fixnums,
   characters, booleans, null, and end-of-file, but soon there will be
   others.
*/
typedef long rs_object;

static inline int rs_immediate_p(rs_object obj);
static inline int rs_heap_p(rs_object obj);


/* In general, each type will have 3 functions. For a type X, the functions are:
   * rs_X_p(rs_object obj) -- checks if obj is of type X.
   * rs_X_to_obj(rs_X val) -- make val into an rs_object.
   * rs_obj_to_X(rs_object obj) -- get a value of type X from obj.
*/


/** Fixnums **/
typedef long rs_fixnum;

/* The minimum and maximum values for fixnums. */
extern const long rs_fixnum_min;
extern const long rs_fixnum_max;

static inline int rs_fixnum_p(rs_object obj);
static inline rs_object rs_fixnum_to_obj(rs_fixnum val);
static inline rs_fixnum rs_obj_to_fixnum(rs_object obj);


/** Characters **/

/* Assume that sizeof(short) >= 2. */
typedef short rs_character;

static inline int rs_character_p(rs_object obj);
static inline rs_object rs_character_to_obj(rs_character val);
static inline rs_character rs_obj_to_character(rs_object obj);


/** Booleans, null, and end-of-file **/
/* Since there are only two boolean values, and a single value each for null
   and end-of-file, there's no need for rs_X_make and rs_X_value functions.
   Instead, each object is just declared directly.
*/
extern const rs_object rs_true;
extern const rs_object rs_false;
extern const rs_object rs_null;
extern const rs_object rs_eof;

static inline int rs_boolean_p(rs_object obj);
static inline int rs_null_p(rs_object obj);
static inline int rs_eof_p(rs_object obj);


/* Symbols, strings, lists, etc. are all heap objects. They have an additional
   function:
   * rs_object rs_X_create(...)
       Allocate and initialize an rs_X. The arguments depend on the type: a
       const char* for rs_symbol or rs_string, for example.
*/

struct rs_hobject;

/* Perform any type-specific cleanup required for obj. */
void rs_hobject_release(struct rs_hobject *obj);


/** Symbols **/
typedef struct rs_hobject rs_symbol;

static inline int rs_symbol_p(rs_object obj);
static inline rs_object rs_symbol_to_obj(rs_symbol *sym);
static inline rs_symbol *rs_obj_to_symbol(rs_object obj);
rs_object rs_symbol_create(const char *name);

/* Get the C string representation of sym. */
static inline const char *rs_symbol_cstr(rs_symbol *sym);


/** Strings **/
typedef struct rs_hobject rs_string;

static inline int rs_string_p(rs_object obj);
static inline rs_object rs_string_to_obj(rs_string *str);
static inline rs_string *rs_obj_to_string(rs_object obj);
rs_object rs_string_create(const char *cstr);

/* Get the C string representation of str. */
static inline char *rs_string_cstr(rs_string *str);



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



/**** gc.c - memory allocation and garbage collection. ****/

/* Initialize the heap and GC. */
void rs_gc_init(void);

/* Release all of the resources used by every object, and free the memory
   used for the heap.
*/
void rs_gc_shutdown(void);

/* Allocate an object on the heap. */
struct rs_hobject *rs_hobject_alloc(void);



/**** symtab.c - symbol table. ****/

/* Add a symbol to the table. Used by rs_symbol_create(). */
const char *rs_symtab_insert(const char *sym);

/* Remove a symbol from the table. Used by rs_object_release(). */
void rs_symtab_remove(const char *sym);



/**** buffer.c - character buffer data structure. ****/

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

/* Returns the C string held in buf. Guaranteed to be NUL-terminated.
   NOTE: The string may be modified after it is returned. If you're going to
   keep it around for long, make a copy and use that instead.
 */
const char *rs_buf_cstr(struct rs_buf *buf);



/**** stack.c - generic stack data structure. ****/

/* A stack frame. Stack functions take pointers to these, and a NULL pointer
   means an empty stack. */
struct rs_stack;

/* Push data onto stack, and return the new top of the stack. */
struct rs_stack *rs_stack_push(struct rs_stack *stack, void *data);

/* Pop the top value of stack, and set stack to point to the new top frame. */
void *rs_stack_pop(struct rs_stack **stack);



/**** error.c - error-reporting. ****/

/* The error macros have a printf-like interface. If the format string ends with
   a ':', then the result of strerror(3) is appended. The "_s" varieties take an
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



/**** End of public API. ***/

#define _RESCHEME_INSIDE_
#include "rescheme_p.h"
#undef  _RESCHEME_INSIDE_

#endif
