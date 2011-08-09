#ifndef _RESCHEME_INSIDE_
#error "Include \"rescheme.h\" instead."
#endif

#ifndef _RESCHEME_P_H
#define _RESCHEME_P_H

/* Inline function defintions, and declarations that need to be globally
   visible, but should not be directly used.
*/

/**** object.c ****/
#define _TAG_BITS 2
#define _TAG_MASK 3


#define _FIXNUM_TAG 1

static inline int rs_fixnum_p(rs_object obj) {
	return ((rs_fixnum)obj & _TAG_MASK) == _FIXNUM_TAG;
}

static inline rs_object rs_fixnum_to_obj(rs_fixnum val) {
	return (rs_object)(val << _TAG_BITS) + _FIXNUM_TAG;
}

static inline rs_fixnum rs_obj_to_fixnum(rs_object obj) {
	return (rs_fixnum)(obj >> _TAG_BITS);
}


#define _CHARACTER_TAG 2

static inline int rs_character_p(rs_object obj) {
	return ((rs_character)obj & _TAG_MASK) == _CHARACTER_TAG;
}

static inline rs_object rs_character_to_obj(rs_character val) {
	return (rs_object)(val << _TAG_BITS) + _CHARACTER_TAG;
}

static inline rs_character rs_obj_to_character(rs_object obj) {
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


/**** buffer.c ****/
struct rs_buf {
	char *buf;
	size_t off;
	size_t cap;
};


/**** error.c ****/
void eprintf(int status, char const * const func, char const * const fmt, ...);

#endif
