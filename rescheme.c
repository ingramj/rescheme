#include "rescheme.h"

int main(void)
{
	printf("ReScheme v0.1\n");

	rs_object obj;
	for (;;) {
		printf("> ");
		obj = rs_read(stdin);
		if (obj == EOF) break;
		obj = rs_eval(obj);
		rs_write(stdout, obj);
		printf("\n");
	}
	return 0;
}
