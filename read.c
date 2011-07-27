#include "rescheme.h"
#include <ctype.h>

/* From what I can tell, the normal way to parse Lisp is with a recursive
   descent parser. I'm taking a different approach, and hand-writing a state
   machine. This may end up being a bad idea.
*/


#define BUFSIZE 1024


/* I don't know if these defines are awesome, or abominations, but they do
   make the state machine code more concise.
*/

/* They call it whitespace, but on my screen it's black...  */
#define WS \
	' ': case '\t': case '\r': case '\n'

#define SEP WS

#define DIGIT \
	'0': case '1': case '2': case '3': case '4': \
case '5': case '6': case '7': case '8': case '9'


enum state { ST_START, ST1, ST_END };


rs_object rs_read(FILE *in)
{
	int c;
	char buf[BUFSIZE];
	int off = 0;
	rs_object obj;
	enum state cur_state = ST_START;

	while (cur_state != ST_END) {
		c = getc(in);

		switch (cur_state) {
		case ST_START:
			switch (c) {
			case WS:
				cur_state = ST_START;
				break;
			case DIGIT:
				buf[off++] = c;
				cur_state = ST1;
				break;
			default:
				rs_fatal("expected whitespace or digit");
			}
			break;
		case ST1:
			switch (c) {
			case DIGIT:
				buf[off++] = c;
				cur_state = ST1;
				break;
			case SEP:
				ungetc(c, in);
				buf[off] = '\0';
				obj = rs_fixnum_make(strtol(buf, NULL, 10));
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected digit or separator");
			}
			break;
		default:
			rs_fatal("got into an impossible state");
		}
	}
	return obj;
}
