#include "rescheme.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

/* The ReScheme parser is a state machine. It consists of a large switch inside
   of a loop. The switch has a case for each state that the parser can be in. A
   new input character is read each time the loop executes, and control jumps
   to the current state's case. Then the input character is used to determine
   what the next state should be. The loop ends when the current state is
   ST_END.

   NOTE: Later, when lists are added to ReScheme, the parser will have a stack
   to handle recursion. That will make it a pushdown automata, and it will be
   equivalent to the recursive descent parsers that are typically used to parse
   Lisps. Switching states is analogous to calling functions, except 1) they
   don't return to the previous state when they're finished (no call stack),
   and 2) a new character is read in automatically at each state change.
*/
enum state { ST_START, ST_DECIMAL, ST_HASH, ST_BINARY, ST_OCTAL, ST_HEX,
             ST_CHARACTER, ST_CHAR_N, ST_CHAR_S, ST_CHAR_T, ST_SYMBOL,
             ST_STRING, ST_ESCAPE, ST_END };

/* More can happen inside a state than just choosing the next state. The input
   character can be pushed back so that the next state (or the next call to the
   parser) will see it.
 */
#define PUSH_BACK(c, in) \
	if (c != EOF && ungetc(c, in) == EOF) \
		rs_fatal("ungetc failed:");

/* Characters can also be pushed into a buffer. This can be done to gather the
   digits of a number, or the characters of a symbol.
*/
#define BUF_PUSH(buf, c) \
	if (rs_buf_push((buf), (c)) == NULL) \
		rs_fatal("could not write to buffer:");

/* This fuction turns a buffer full of digits into a ReScheme fixnum. */
static inline rs_object rs_read_check_num(struct rs_buf *buf, int base);

/* Sometimes it's helpful to take a shortcut, and read in several characters at
   once. This function reads characters into a buffer, starting with c, and
   reading the rest from in. It stops when either n characters have been read,
   or it reads in a delimiter (see below).
*/
static void rs_read_get_word(struct rs_buf *buf, FILE *in, int c, int n);

/* Inside each state (most of them, anyway) is an inner switch that checks the
   input character to determine what actions to take. Since many character will
   result in the same action, it makes sense to group them together. These
   macros are ugly, but they make the switches much more concise and legible.
*/
#define WS \
	' ': case '\t': case '\r': case '\n'
#define DELIM WS: case ';'
#define BIN_DIGIT '0': case '1'
#define OCT_DIGIT \
	BIN_DIGIT: case '2': case '3': case '4': case '5': case '6': case '7'
#define DIGIT \
	OCT_DIGIT: case '8': case '9'
#define HEX_DIGIT \
	DIGIT: case 'a': case 'A': case 'b': case 'B': case 'c': case 'C': \
	case 'd': case 'D': case 'e': case 'E': case 'f': case 'F'
#define LC_LETTER \
	'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': \
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': \
	case 'v': case 'w': case 'x': case 'y': case 'z'
#define UC_LETTER \
	'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': \
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': \
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': \
	case 'V': case 'W': case 'X': case 'Y': case 'Z'
#define SYMBOL_INIT \
	LC_LETTER: case UC_LETTER: case '!': case '$': case '%': case '*': \
	case '/': case ':': case '<': case '=': case '>': case '?': case '^': \
	case '_': case '~'
#define SYMBOL_SUB \
	SYMBOL_INIT: case DIGIT: case '+': case '-': case '.': case '@'


/* The parser function. It reads characters from in, and turns them into an
   object. Or it dies when there's a syntax error.
*/
rs_object rs_read(FILE *in)
{
	assert(in != NULL);

	/* When an object is recognized, it is saved in obj, and then the state is
	   changed to ST_END.
	*/
	rs_object obj = rs_eof;

	/* Changing states is done by assigning to cur_state, and then breaking out
	   of the switch.
	*/
	enum state cur_state = ST_START;

	/* The buffer is used to store characters, so that they can be used when
	   creating objects. Buffers have to be initialized before use.
	 */
	struct rs_buf buf;
	rs_buf_init(&buf);


	while (cur_state != ST_END) {
		int c = getc(in);

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
				obj = rs_eof;
				cur_state = ST_END;
				break;
			case DIGIT:
				BUF_PUSH(&buf, c);
				cur_state = ST_DECIMAL;
				break;
			case '+': case '-':
				BUF_PUSH(&buf, c);
				/* Look ahead to see if it's a number or a symbol. */
				c = getc(in);
				switch (c) {
				case DIGIT:
					BUF_PUSH(&buf, c);
					cur_state = ST_DECIMAL;
					break;
				case DELIM:
					PUSH_BACK(c, in);
					obj = rs_symbol_create(rs_buf_cstr(&buf));
					cur_state = ST_END;
					break;
				default:
					rs_fatal("expected a digit or a delimiter");
				}
				break;
			case '#':
				cur_state = ST_HASH;
				break;
			case '(':
				c = getc(in);
				if (c == ')') {
					obj = rs_null;
					cur_state = ST_END;
				} else {
					rs_fatal("non-empty lists have not been implemented");
				}
				break;
			case SYMBOL_INIT:
				BUF_PUSH(&buf, tolower(c));
				cur_state = ST_SYMBOL;
				break;
			case '"':
				cur_state = ST_STRING;
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
			case DELIM:
				PUSH_BACK(c, in);
				obj = rs_read_check_num(&buf, 10);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected a digit, or a delimiter");
			}
			break;

		case ST_HASH: {
			/* Lots of things can start with a # in Scheme. Most of the cases
			   are for numbers, which need a little extra work at the end of
			   this state. This flag is set to zero if that work isn't needed.
			*/
			int is_fixnum = 1;
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
			case '\\':
				cur_state = ST_CHARACTER;
				is_fixnum = 0;
				break;
			case 't': case 'T':
				obj = rs_true;
				is_fixnum = 0;
				cur_state = ST_END;
				break;
			case 'f': case 'F':
				obj = rs_false;
				is_fixnum = 0;
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected a radix, or a character literal");
			}
			/* If we're expecting a fixnum, look ahead to see if the next
			   character is a + or -. */
			if (is_fixnum) {
				c = getc(in);
				switch (c) {
				case '+': case '-':
					BUF_PUSH(&buf, c);
					break;
				case DELIM:
					rs_fatal("expected a digit");
				default:
					PUSH_BACK(c, in);
				}
			}
		}
			break;

		case ST_BINARY:
			switch (c) {
			case BIN_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case DELIM:
				PUSH_BACK(c, in);
				obj = rs_read_check_num(&buf, 2);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected a binary digit, or a delimiter");
			}
			break;

		case ST_OCTAL:
			switch (c) {
			case OCT_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case DELIM:
				PUSH_BACK(c, in);
				obj = rs_read_check_num(&buf, 8);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected an octal digit, or a delimiter");
			}
			break;

		case ST_HEX:
			switch (c) {
			case HEX_DIGIT:
				BUF_PUSH(&buf, c);
				break;
			case DELIM:
				PUSH_BACK(c, in);
				obj = rs_read_check_num(&buf, 16);
				cur_state = ST_END;
				break;
			default:
				rs_fatal("expected a hex digit, or a delimiter");
			}
			break;

		case ST_CHARACTER:
			/* If the character starts with an 'n', 's', or 't', then we need
			   to see if its actually a 'newline', 'space', or 'tab'. That's
			   handled by other states.
			*/
			switch (c) {
			case 'n': case 'N':
				PUSH_BACK(c, in);
				cur_state = ST_CHAR_N;
				break;
			case 's': case 'S':
				PUSH_BACK(c, in);
				cur_state = ST_CHAR_S;
				break;
			case 't': case 'T':
				PUSH_BACK(c, in);
				cur_state = ST_CHAR_T;
				break;
			default:
				/* Otherwise, read in a character, and make sure that it's
				   followed by a delimiter. */
				if (isgraph(c)) {
					char d = getc(in);
					switch(d) {
					case DELIM:
						PUSH_BACK(d, in);
						obj = rs_character_to_obj(c);
						cur_state = ST_END;
						break;
					default:
						rs_fatal("unknown character literal");
					}
				} else {
					rs_fatal("expected a character literal");
				}
			}
			break;

		case ST_CHAR_N:
			assert(c == 'n' || c == 'N');
			/* See if we have #\newline. */
			rs_read_get_word(&buf, in, c, 7);
			if (strcmp("n", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj(c);
			} else if (strcmp("newline", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj('\n');
			} else {
				rs_fatal("unknown character literal (#\\%s)",
				         rs_buf_cstr(&buf));
			}
			cur_state = ST_END;
			break;

		case ST_CHAR_S:
			assert(c == 's' || c == 'S');
			/* See if we have #\space. */
			rs_read_get_word(&buf, in, c, 5);
			if (strcmp("s", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj(c);
			} else if (strcmp("space", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj(' ');
			} else {
				rs_fatal("unknown character literal (#\\%s)",
				         rs_buf_cstr(&buf));
			}
			cur_state = ST_END;
			break;

		case ST_CHAR_T:
			assert(c == 't' || c == 'T');
			/* See if we have #\tab (which is non-standard). */
			rs_read_get_word(&buf, in, c, 3);
			if (strcmp("t", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj(c);
			} else if (strcmp("tab", rs_buf_cstr(&buf)) == 0) {
				obj = rs_character_to_obj('\t');
			} else {
				rs_fatal("unknown character literal (#\\%s)",
				         rs_buf_cstr(&buf));
			}
			cur_state = ST_END;
			break;

		case ST_SYMBOL:
			switch (c) {
			case SYMBOL_SUB:
				BUF_PUSH(&buf, tolower(c));
				break;
			case DELIM:
				PUSH_BACK(c, in);
				obj = rs_symbol_create(rs_buf_cstr(&buf));
				cur_state = ST_END;
				break;
			default:
				if (isgraph(c)) {
					rs_fatal("'%c' cannot appear in an identifier", c);
				} else {
					rs_fatal("'\\x%02x' cannot appear in an identifier", c);
				}
			}
			break;

		case ST_STRING:
			switch (c) {
			case '"':
				obj = rs_string_create(rs_buf_cstr(&buf));
				cur_state = ST_END;
				break;
			case '\\':
				cur_state = ST_ESCAPE;
				break;
			default:
				BUF_PUSH(&buf, c);
			}
			break;

		case ST_ESCAPE:
			switch (c) {
			case 'n':
				BUF_PUSH(&buf, '\n');
				break;
			case 't':
				BUF_PUSH(&buf, '\t');
				break;
			case '"':
				BUF_PUSH(&buf, '"');
				break;
			case '\\':
				BUF_PUSH(&buf, '\\');
				break;
			case 'r':
				BUF_PUSH(&buf, '\r');
				break;
			case 'b':
				BUF_PUSH(&buf, '\b');
				break;
			case 'a':
				BUF_PUSH(&buf, '\a');
				break;
			default:
				if (isgraph(c)) {
					rs_fatal("unknown string escape sequence: \\%c", c);
				} else {
					rs_fatal("unknown string escape sequence: \\x%02x", c);
				}
			}
			cur_state = ST_STRING;
			break;

		default:
			rs_fatal("got into an impossible state");
		}
	}

	rs_buf_reset(&buf);
	return obj;
}


static inline rs_object rs_read_check_num(struct rs_buf *buf, int base)
{
	assert(buf != NULL);
	assert(base == 10 || base == 2 || base == 8 || base == 16);

	long value = strtol(rs_buf_cstr(buf), NULL, base);
	if (value < rs_fixnum_min || value > rs_fixnum_max) {
		rs_fatal("number out of range");
	}
	return rs_fixnum_to_obj(value);
}


static void rs_read_get_word(struct rs_buf *buf, FILE *in, int c, int n)
{
	assert(buf != NULL);
	assert(in != NULL);

	BUF_PUSH(buf, tolower(c));
	int loop = 1;
	while (loop) {
		c = getc(in);
		switch (c) {
		case DELIM:
			loop = 0;
			PUSH_BACK(c, in);
			break;
		case EOF:
			rs_fatal("unexpected EOF");
		default:
			BUF_PUSH(buf, tolower(c));
		}
		if (loop && (--n <= 0)) break;
	}
}
