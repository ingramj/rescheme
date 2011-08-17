#include "rescheme.h"

int main(void)
{
	printf("ReScheme v0.2\n");

	rs_gc_init();

	rs_object obj;

	obj = rs_string_create("Hello, World!\n");
	rs_write(stdout, obj);

	for (;;) {
		printf("> ");
		obj = rs_read(stdin);
		if (rs_eof_p(obj)) break;
		obj = rs_eval(obj);
		rs_write(stdout, obj);
		printf("\n");
	}

	rs_gc_shutdown();
	return 0;
}
