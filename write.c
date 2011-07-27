#include "rescheme.h"


int rs_write(FILE *out, rs_object obj)
{
	return fprintf(out, "%ld", (long)rs_fixnum_value(obj));
}
