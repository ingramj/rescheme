#include "rescheme.h"

int main(void)
{
	printf("ReScheme v0.2\n");

	rs_object obj;
	for (;;) {
		printf("> ");
		obj = rs_read(stdin);
		if (rs_eof_p(obj)) break;
		obj = rs_eval(obj);
		rs_write(stdout, obj);
		printf("\n");

		if (rs_symbol_p(obj)) {
			rs_symbol_release(rs_obj_to_symbol(obj));
		}
	}
	return 0;
}
