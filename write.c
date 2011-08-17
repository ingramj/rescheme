#include "rescheme.h"

#include <assert.h>
#include <ctype.h>


static int rs_write_string(FILE *out, rs_string *str);


int rs_write(FILE *out, rs_object obj)
{
	assert(out != NULL);

	int result;
	if (rs_fixnum_p(obj)) {
		result = fprintf(out, "%ld", (long)rs_obj_to_fixnum(obj));
	} else if (rs_character_p(obj)) {
		rs_character c = rs_obj_to_character(obj);
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
	} else if (rs_symbol_p(obj)) {
		result = fprintf(out, "%s", rs_symbol_cstr(rs_obj_to_symbol(obj)));
	} else if (rs_string_p(obj)) {
		result = rs_write_string(out, rs_obj_to_string(obj));
	} else {
		rs_fatal("illegal object type");
	}
	return result;
}


static int rs_write_string(FILE *out, rs_string *str)
{
	assert(rs_string_cstr(str) != NULL);

	const char *cstr = rs_string_cstr(str);
	int count = 0;
	fputc('"', out);
	while (*cstr != '\0') {
		switch (*cstr) {
		case '\n':
			count += fprintf(out, "\\n");
			break;
		case '\t':
			count += fprintf(out, "\\t");
			break;
		case '\"':
			count += fprintf(out, "\\\"");
			break;
		case '\\':
			count += fprintf(out, "\\\\");
			break;
		case '\r':
			count += fprintf(out, "\\r");
			break;
		case '\b':
			count += fprintf(out, "\\b");
			break;
		case '\a':
			count += fprintf(out, "\\a");
			break;
		case ' ':
			fputc(' ', out);
			count++;
			break;
		default:
			if (isgraph(*cstr)) {
				fputc(*cstr, out);
				count++;
			} else {
				count += fprintf(out, "\\x%02x", *cstr);
			}
		}
		cstr++;
	}
	fputc('"', out);
	return 2 + count;
}
