#include "rescheme.h"


#define HEAP_SIZE 1024


static struct rs_hobject *heap = NULL;
static int next_obj = 0;


static struct rs_stack *root = NULL;


static int rs_gc_next_obj(void);
static void rs_gc_mark(void);
static void rs_gc_mark_obj(struct rs_hobject *obj);
static void rs_gc_sweep(void);


#define GC_FLAG_ALLOC_P(flags) ((flags) & 1)
#define GC_FLAG_ALLOC_SET(flags) ((flags) |= 1)
#define GC_FLAG_ALLOC_CLEAR(flags) ((flags) &= ~1)

#define GC_FLAG_MARK_P(flags) ((flags) & 2)
#define GC_FLAG_MARK_SET(flags) ((flags) |= 2)
#define GC_FLAG_MARK_CLEAR(flags) ((flags) &= ~2)


void rs_gc_init(void)
{
	TRACE("heap size = %ld objects", HEAP_SIZE);
	heap = calloc(HEAP_SIZE, sizeof(struct rs_hobject));
	if (heap == NULL) {
		rs_fatal("cannot allocate heap:");
	}
}


void rs_gc_shutdown(void)
{
	assert(heap != NULL);
	for (int i = 0; i < HEAP_SIZE; i++) {
		if (GC_FLAG_ALLOC_P(heap[i].flags)) {
			rs_hobject_release(&(heap[i]));
		}
	}
	free(heap);
}


struct rs_hobject *rs_gc_alloc_hobject(void)
{
	assert(heap != NULL);

	int i;
	if ((i = rs_gc_next_obj()) < 0) {
		TRACE("garbage day");
		rs_gc_mark();
		rs_gc_sweep();
		if ((i = rs_gc_next_obj()) < 0) {
			rs_fatal("could not allocate an object");
		}
	}

	GC_FLAG_ALLOC_SET(heap[i].flags);
	GC_FLAG_MARK_CLEAR(heap[i].flags);
	return &(heap[i]);
}


void rs_gc_push(struct rs_hobject *obj)
{
	assert(obj != NULL);
	root = rs_stack_push(root, obj);
}


struct rs_hobject *rs_gc_pop(void)
{
	assert(root != NULL);
	assert(rs_stack_top(root) != NULL);

	return (struct rs_hobject*)rs_stack_pop(&root);
}


static int rs_gc_next_obj(void)
{
	assert(next_obj >= 0 && next_obj < HEAP_SIZE);

	int i = next_obj;
	for (;;) {
		if (!GC_FLAG_ALLOC_P(heap[i].flags)) {
			next_obj = (i + 1) % HEAP_SIZE;
			return i;
		}
		i = (i + 1) % HEAP_SIZE;
		if (i == next_obj) {
			return -1;
		}
	}
}


static void rs_gc_mark(void)
{
	struct rs_stack *s = root;
	while (s != NULL) {
		rs_gc_mark_obj(rs_stack_top(s));
		s = s->next;
	}
	return;
}


static void rs_gc_mark_obj(struct rs_hobject *obj)
{
	assert(obj != NULL);

	GC_FLAG_MARK_SET(obj->flags);
	if (rs_pair_p((rs_object)obj)) {
		if (rs_heap_p(rs_pair_car(obj))) {
			rs_gc_mark_obj((struct rs_hobject*) rs_pair_car(obj));
		}
		if (rs_heap_p(rs_pair_cdr(obj))) {
			rs_gc_mark_obj((struct rs_hobject*) rs_pair_cdr(obj));
		}
	}
}


void rs_gc_sweep(void)
{
	assert(heap != NULL);

	for (int i = 0; i < HEAP_SIZE; i++) {
		if (GC_FLAG_MARK_P(heap[i].flags)) {
			GC_FLAG_MARK_CLEAR(heap[i].flags);
		} else {
			GC_FLAG_ALLOC_CLEAR(heap[i].flags);
			rs_hobject_release(&(heap[i]));
		}
	}
}
