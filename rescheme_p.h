#ifndef _RESCHEME_INSIDE_
#error Include "rescheme.h" instead.
#endif

#ifndef _RESCHEME_P_H
#define _RESCHEME_P_H

#include <assert.h>

/* Inline function defintions, and declarations that need to be globally
   visible, but should not be directly used.
*/

/**** object.c ****/
#define _TAG_BITS 2
#define _TAG_MASK 3

#define _HOBJECT_TAG 0
#define _FIXNUM_TAG 1
#define _CHARACTER_TAG 2


static inline int rs_immediate_p(rs_object obj) {
	return (obj & _TAG_MASK) != _HOBJECT_TAG;
}

static inline int rs_heap_p(rs_object obj) {
	assert((void*)obj != NULL);
	return (obj & _TAG_MASK) == _HOBJECT_TAG;
}

static inline int rs_fixnum_p(rs_object obj) {
	return ((rs_fixnum)obj & _TAG_MASK) == _FIXNUM_TAG;
}

static inline rs_object rs_fixnum_to_obj(rs_fixnum val) {
	return (rs_object)(val << _TAG_BITS) + _FIXNUM_TAG;
}

static inline rs_fixnum rs_obj_to_fixnum(rs_object obj) {
	assert(rs_fixnum_p(obj));
	return (rs_fixnum)(obj >> _TAG_BITS);
}

static inline int rs_character_p(rs_object obj) {
	return ((rs_character)obj & _TAG_MASK) == _CHARACTER_TAG;
}

static inline rs_object rs_character_to_obj(rs_character val) {
	return (rs_object)(val << _TAG_BITS) + _CHARACTER_TAG;
}

static inline rs_character rs_obj_to_character(rs_object obj) {
	assert(rs_character_p(obj));
	return (rs_character)(obj >> _TAG_BITS);
}

static inline int rs_boolean_p(rs_object obj) {
	return obj == rs_true || obj == rs_false;
}

static inline int rs_null_p(rs_object obj) {
	return obj == rs_null;
}

static inline int rs_eof_p(rs_object obj) {
	return obj == rs_eof;
}


enum rs_hobject_type {
	RS_SYMBOL, RS_STRING
};

struct rs_hobject {
	enum rs_hobject_type type;
	union {
		const char *sym;
		char *str;
	} val;
	char flags;
};

static inline int rs_symbol_p(rs_object obj) {
	return rs_heap_p(obj) && ((struct rs_hobject*)obj)->type == RS_SYMBOL;
}

static inline rs_object rs_symbol_to_obj(rs_symbol *sym) {
	assert(sym != NULL);
	assert(sym->type == RS_SYMBOL);
	return (rs_object)sym;
}

static inline rs_symbol *rs_obj_to_symbol(rs_object obj) {
	assert(rs_symbol_p(obj));
	return (rs_symbol*)obj;
}

static inline const char *rs_symbol_cstr(rs_symbol *sym) {
	assert(sym != NULL);
	assert(sym->type == RS_SYMBOL);
	assert(sym->val.sym != NULL);
	return sym->val.sym;
}

static inline int rs_string_p(rs_object obj) {
	return rs_heap_p(obj) && ((struct rs_hobject*)obj)->type == RS_STRING;
}

static inline rs_object rs_string_to_obj(rs_string *str) {
	assert(str != NULL);
	assert(str->type == RS_STRING);
	return (rs_object)str;
}

static inline rs_string *rs_obj_to_string(rs_object obj) {
	assert(rs_string_p(obj));
	return (rs_string*)obj;
}

static inline char *rs_string_cstr(rs_string *str) {
	assert(str != NULL);
	assert(str->type == RS_STRING);
	assert(str->val.str != NULL);
	return str->val.str;
}


/**** buffer.c ****/
struct rs_buf {
	char *buf;
	size_t off;
	size_t cap;
};


/**** error.c ****/
void eprintf(int status, char const * const func, char const * const fmt, ...);

#endif
