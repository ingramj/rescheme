#include "rescheme.h"


int rs_write(FILE *out, rs_object obj)
{
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
	} else {
		rs_fatal("illegal object type");
	}
	return result;
}
