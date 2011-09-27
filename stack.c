#include "rescheme.h"

#include <assert.h>

static struct rs_stack *rs_stack_alloc_frame(void *data);

struct rs_stack *rs_stack_push(struct rs_stack *stack, void *data)
{
	struct rs_stack *new_top = rs_stack_alloc_frame(data);
	if (stack != NULL) {
		new_top->next = stack;
	}
	return new_top;
}

void *rs_stack_pop(struct rs_stack **stack)
{
	assert(stack != NULL);
	assert(*stack != NULL);

	struct rs_stack *old_top = *stack;
	*stack = old_top->next;
	void *data = old_top->data;

	free(old_top);
	return data;
}


static struct rs_stack *rs_stack_alloc_frame(void *data)
{
	struct rs_stack *frame = malloc(sizeof(struct rs_stack));
	if (frame == NULL) {
		rs_fatal("could not allocate stack frame:");
	}
	frame->data = data;
	frame->next = NULL;
	return frame;
}
