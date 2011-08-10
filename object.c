#include "rescheme.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>


const long rs_fixnum_min = ((((rs_fixnum)1) << (8 * sizeof(rs_object) - 1)) >>
                            _TAG_BITS) + 1;

/* It would be nicer to just say rs_fixnum_max = -rs_fixnum_min, but not all
   compilers allow it. */
const long rs_fixnum_max = -(((((rs_fixnum)1) << (8 * sizeof(rs_object) - 1)) >>
                              _TAG_BITS) + 1);


const rs_object rs_true  = 3;   // 0011
const rs_object rs_false = 7;   // 0111
const rs_object rs_null  = 11;  // 1011
const rs_object rs_eof   = 15;  // 1111


/* Allocate and free heap objects. */
static struct rs_hobject *rs_alloc_obj(void);
static void rs_free_obj(struct rs_hobject *obj);


static void rs_symbol_release(rs_symbol *sym);

void rs_object_release(rs_object obj)
{
	if (rs_immediate_p(obj)) return;

	if (rs_symbol_p(obj)) {
		rs_symbol_release(rs_obj_to_symbol(obj));
	} else {
		rs_fatal("unknown object type");
	}
	rs_free_obj((struct rs_hobject*)obj);
}


rs_object rs_symbol_create(const char *name)
{
	assert(name != NULL);
	rs_symbol *sym = rs_alloc_obj();
	sym->type = RS_SYMBOL;
	sym->val.sym = strdup(name);
	if (sym->val.sym == NULL) {
		rs_fatal("could not allocate symbol:");
	}
	return rs_symbol_to_obj(sym);
}


static void rs_symbol_release(rs_symbol *sym)
{
	assert(sym != NULL);
	assert(sym->val.sym != NULL);
	assert(rs_symbol_p((rs_object)sym));

	free(sym->val.sym);
}


static struct rs_hobject *rs_alloc_obj(void)
{
	struct rs_hobject *obj = malloc(sizeof(struct rs_hobject));
	if (obj == NULL) {
		rs_fatal("could not allocate heap object:");
	}
	return obj;
}


static void rs_free_obj(struct rs_hobject *obj)
{
	assert(obj != NULL);
	free(obj);
}
