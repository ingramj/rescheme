#include "rescheme.h"

static void stack_test(void);

int main(void)
{
	printf("ReScheme v0.3\n");

#ifdef DEBUG
	stack_test();
#endif

	rs_gc_init();

	rs_object obj;
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


static void stack_test(void)
{
	struct rs_stack *stack = NULL;
	stack = rs_stack_push(stack, (void *) 1);
	stack = rs_stack_push(stack, (void *) 2);
	stack = rs_stack_push(stack, (void *) 3);
	for (int i = 3; i > 0; i--) {
		int data = (int) rs_stack_pop(&stack);
		if (data != i) {
			rs_fatal("expected %d, got %d", i, data);
		}
	}
	TRACE("passed.");
}
