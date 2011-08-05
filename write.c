#include "rescheme.h"

#include <assert.h>


int rs_write(FILE *out, rs_object obj)
{
	assert(out != NULL);

	int result;
	if (rs_fixnum_p(obj)) {
		result = fprintf(out, "%ld", (long)rs_fixnum_value(obj));
	} else if (rs_character_p(obj)) {
		rs_character c = rs_character_value(obj);
		if ((char)c == '\n') {
			result = fprintf(out, "#\\newline");
		} else if ((char)c == '\t') {
			result = fprintf(out, "#\\tab");
		} else if ((char)c == ' ') {
			result = fprintf(out, "#\\space");
		} else {
			result = fprintf(out, "#\\%c", (char)c);
		}
	} else if (rs_boolean_p(obj)) {
		result = fprintf(out, "#%c", obj == rs_true ? 't' : 'f');
	} else if (rs_null_p(obj)) {
		result = fprintf(out, "()");
	} else {
		rs_fatal("illegal object type");
	}
	return result;
}
