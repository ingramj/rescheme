#include "rescheme.h"
#include <ctype.h>
#include <limits.h>

/* From what I can tell, the normal way to parse Lisp is with a recursive
   descent parser. I'm taking a different approach, and hand-writing a
   pushdown automata-based parser.
*/


/* Turn the string in buf into an rs_fixnum, or die trying. */
static inline rs_object check_num(struct rs_buf *buf, int base);


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


#ifdef DEBUG
# define PUSH_BACK(c, in) \
	if (c != EOF && ungetc(c, in) == EOF) \
		rs_fatal("state %d: ungetc failed:", cur_state)
# define BUF_PUSH(buf, c) \
	if (rs_buf_push((buf), (c)) == NULL) \
		rs_fatal("state %d: could not write to buffer:", cur_state);
#else
# define PUSH_BACK(c, in) \
	if (c != EOF && ungetc(c, in) == EOF) \
		rs_fatal("ungetc failed:");
# define BUF_PUSH(buf, c) \
	if (rs_buf_push((buf), (c)) == NULL) \
		rs_fatal("could not write to buffer:");
#endif


enum state { ST_START, ST_DECIMAL, ST_HASH, ST_BINARY, ST_OCTAL,
             ST_HEX, ST_END };


rs_object rs_read(FILE *in)
{
	int c;
	struct rs_buf buf;
	rs_object obj;
	enum state cur_state = ST_START;

	rs_buf_init(&buf);

	while (cur_state != ST_END) {
		c = getc(in);

		switch (cur_state) {
		case ST_START:
			switch (c) {
			case WS:
				/* skip whitespace */
				break;
			case ';':
				/* skip comments */
				while ((c = getc(in)) != '\n' && c != EOF) ;
				break;
			case EOF:
				obj = EOF;  // Eventually we'll have an actual eof object.
				cur_state = ST_END;
				break;
			case DIGIT:
				BUF_PUSH(&buf, c);
				cur_state = ST_DECIMAL;
				break;
			case '+': case '-':
				BUF_PUSH(&buf, c);
				/* Look ahead to see if it's followed by a digit. */
				c = getc(in);
				switch (c) {
				case DIGIT:
					BUF_PUSH(&buf, c);
					cur_state = ST_DECIMAL;
					break;
				default:
					rs_fatal("expected a digit");
				}
				break;
			case '#':
				cur_state = ST_HASH;
				break;
			default:
				rs_fatal("invalid expression");
			}
			break;

		case ST_DECIMAL:
			switch (c) {
			case DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case SEP:
				PUSH_BACK(c, in);
				obj = check_num(&buf, 10);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected digit or separator");
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
			case 'x': case 'X':
				cur_state = ST_HEX;
				break;
			default:
				rs_fatal("expected radix (b, o, d, or h)");
			}
			/* Look ahead to see if it's followed by a + or -. */
			c = getc(in);
			switch (c) {
			case '+': case '-':
				BUF_PUSH(&buf, c);
				break;
			default:
				PUSH_BACK(c, in);
			}
			break;

		case ST_BINARY:
			switch (c) {
			case BIN_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case SEP:
				PUSH_BACK(c, in);
				obj = check_num(&buf, 2);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected binary digit or separator");
			}
			break;

		case ST_OCTAL:
			switch (c) {
			case OCT_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case SEP:
				PUSH_BACK(c, in);
				obj = check_num(&buf, 8);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected octal digit or separator");
			}
			break;

		case ST_HEX:
			switch (c) {
			case HEX_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case SEP:
				PUSH_BACK(c, in);
				obj = check_num(&buf, 16);
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
	rs_buf_reset(&buf);
	return obj;
}


static inline rs_object check_num(struct rs_buf *buf, int base)
{
	long value = strtol(rs_buf_str(buf), NULL, base);
	if (value < rs_fixnum_min || value > rs_fixnum_max) {
		rs_fatal("number out of range");
	}
	return rs_fixnum_make(value);
}
