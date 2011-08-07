#include "rescheme.h"

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
