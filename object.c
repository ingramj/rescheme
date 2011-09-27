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


static void rs_symbol_release(rs_symbol *sym);
static void rs_string_release(rs_string *str);

void rs_hobject_release(struct rs_hobject *obj)
{
	if (rs_symbol_p((rs_object)obj)) {
		rs_symbol_release(obj);
	} else if (rs_string_p((rs_object)obj)) {
		rs_string_release(obj);
	} else {
		rs_fatal("unknown object type");
	}
}


rs_object rs_symbol_create(const char *name)
{
	assert(name != NULL);
	rs_symbol *sym = rs_gc_alloc_hobject();
	sym->type = RS_SYMBOL;
	sym->val.sym = rs_symtab_insert(name);

	return rs_symbol_to_obj(sym);
}


static void rs_symbol_release(rs_symbol *sym)
{
	assert(rs_symbol_p((rs_object)sym));
	assert(sym->val.sym != NULL);

	rs_symtab_remove(sym->val.sym);
}


rs_object rs_string_create(const char *cstr)
{
	assert(cstr != NULL);
	rs_string *str = rs_gc_alloc_hobject();
	str->type = RS_STRING;
	str->val.str = strdup(cstr);
	if (str->val.str == NULL) {
		rs_fatal("could not create string object:");
	}

	return rs_string_to_obj(str);
}


static void rs_string_release(rs_string *str)
{
	assert(rs_string_p((rs_object)str));
	assert(str->val.str != NULL);

	free(str->val.str);
}
