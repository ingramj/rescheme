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

#define BIN_DIGIT '0': case '1'

#define OCT_DIGIT \
	'0': case '1': case '2': case '3': \
	case '4': case '5': case '6': case '7'

#define DIGIT \
	'0': case '1': case '2': case '3': case '4': \
	case '5': case '6': case '7': case '8': case '9'

#define HEX_DIGIT \
	'0': case '1': case '2': case '3': case '4': \
	case '5': case '6': case '7': case '8': case '9': \
	case 'a': case 'A': case 'b': case 'B': case 'c': case 'C': \
	case 'd': case 'D': case 'e': case 'E': case 'f': case 'F'


enum state { ST_START, ST_DECIMAL, ST_SIGNED, ST_HASH, ST_BINARY, ST_OCTAL,
             ST_HEX, ST_END };


rs_object rs_read(FILE *in)
{
	/* FIXME: There are possible buffer overflows in this function. */
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
			case EOF:
				obj = EOF;  // Eventually we'll have an actual eof object.
				cur_state = ST_END;
				break;
			case WS:
				/* skip whitespace */
				break;
			case ';':
				/* skip comments */
				while ((c = getc(in)) != '\n' && c != EOF) ;
				break;
			case DIGIT:
				buf[off++] = c;
				cur_state = ST_DECIMAL;
				break;
			case '+': case '-':
				buf[off++] = c;
				cur_state = ST_SIGNED;
				break;
			case '#':
				cur_state = ST_HASH;
				break;
			default:
				rs_fatal("expected whitespace or digit");
			}
			break;
		case ST_DECIMAL:
			switch (c) {
			case DIGIT:
				buf[off++] = c;
				cur_state = ST_DECIMAL;
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
		case ST_SIGNED:
			switch (c) {
			case DIGIT:
				buf[off++] = c;
				cur_state = ST_DECIMAL;
				break;
			default:
				rs_fatal("expected digit");
			}
			break;
		case ST_HASH:
			switch (c) {
			case 'b': case 'B':
				cur_state = ST_BINARY;
				break;
			case 'o': case 'O':
				cur_state = ST_OCTAL;
				break;
			case 'd': case 'D':
				cur_state = ST_DECIMAL;
				break;
			case 'h': case 'H':
				cur_state = ST_HEX;
				break;
			default:
				rs_fatal("expected radix (b, o, d, or h)");
			}
			break;
		case ST_BINARY:
			switch (c) {
			case BIN_DIGIT:
				buf[off++] = c;
				cur_state = ST_BINARY;
				break;
			case SEP:
				ungetc(c, in);
				buf[off] = '\0';
				obj = rs_fixnum_make(strtol(buf, NULL, 2));
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected binary digit or separator");
			}
			break;
		case ST_OCTAL:
			switch (c) {
			case OCT_DIGIT:
				buf[off++] = c;
				cur_state = ST_OCTAL;
				break;
			case SEP:
				ungetc(c, in);
				buf[off] = '\0';
				obj = rs_fixnum_make(strtol(buf, NULL, 8));
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected octal digit or separator");
			}
			break;
		case ST_HEX:
			switch (c) {
			case HEX_DIGIT:
				buf[off++] = c;
				cur_state = ST_HEX;
				break;
			case SEP:
				ungetc(c, in);
				buf[off] = '\0';
				obj = rs_fixnum_make(strtol(buf, NULL, 16));
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected hex digit or separator");
			}
			break;
		default:
			rs_fatal("got into an impossible state");
		}
	}
	return obj;
}
