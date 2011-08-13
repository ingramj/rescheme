#include "rescheme.h"


#define HEAP_SIZE 1024


static struct rs_hobject *heap = NULL;
static int next_obj = 0;

static int rs_gc_next_obj(void);
static void rs_gc_mark(void);
static void rs_gc_sweep(void);


#define GC_FLAG_ALLOC_P(flags) ((flags) & 1)
#define GC_FLAG_ALLOC_SET(flags) ((flags) |= 1)
#define GC_FLAG_ALLOC_CLEAR(flags) ((flags) &= ~1)

#define GC_FLAG_MARK_P(flags) ((flags) & 2)
#define GC_FLAG_MARK_SET(flags) ((flags) |= 2)
#define GC_FLAG_MARK_CLEAR(flags) ((flags) &= ~2)


void rs_gc_init(void)
{
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


struct rs_hobject *rs_hobject_alloc(void)
{
	assert(heap != NULL);

	int i;
	if ((i = rs_gc_next_obj()) < 0) {
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
	// We have no roots yet, so do nothing.
	return;
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
