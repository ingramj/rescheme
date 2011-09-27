#include "rescheme.h"

#include <assert.h>
#include <string.h>

#define _RS_BUF_GROWBY 128


void rs_buf_init(struct rs_buf *buf)
{
	assert(buf != NULL);

	buf->buf = NULL;
	buf->cap = 0;
	buf->off = 0;
}


void rs_buf_reset(struct rs_buf *buf)
{
	assert(buf != NULL);

	free(buf->buf);
	buf->buf = NULL;
	buf->cap = 0;
	buf->off = 0;
}


struct rs_buf *rs_buf_push(struct rs_buf *buf, char c)
{
	assert(buf != NULL);
	assert((buf->buf == NULL && buf->cap == 0 && buf->off == 0) ||
	       (buf->buf != NULL && buf->cap > 0 && buf->off < buf->cap));

	if (buf->buf == NULL) {
		buf->buf = calloc(_RS_BUF_GROWBY, 1);
		if (buf->buf == NULL) {
			return NULL;
		}
		buf->cap = _RS_BUF_GROWBY;
	}

	if (buf->off >= buf->cap - 1) {
		char *newbuf = realloc(buf->buf, buf->cap + _RS_BUF_GROWBY);
		if (newbuf == NULL) {
			return NULL;
		}
		buf->buf = newbuf;
		buf->cap += _RS_BUF_GROWBY;
	}
	buf->buf[buf->off++] = c;

	assert(buf->buf != NULL);
	assert(buf->cap > 0);
	assert(buf->off > 0 && buf->off < buf->cap);
	return buf;
}


const char *rs_buf_cstr(struct rs_buf *buf)
{
	assert(buf != NULL);
	assert((buf->buf == NULL && buf->cap == 0 && buf->off == 0) ||
	       (buf->buf != NULL && buf->cap > 0 && buf->off < buf->cap));

	if (buf->buf == NULL) {
		return "";
	}
	buf->buf[buf->off] = '\0';
	return (const char *) buf->buf;
}


void rs_buf_test(void)
{
	struct rs_buf buf;
	rs_buf_init(&buf);

	// Make sure the buffer is empty.
	assert(strcmp(rs_buf_cstr(&buf), "") == 0);

	char str[] = "This is the test string.\n";

	// Push the characters of str into buf, excluding the final '\0'.
	for (int i = 0; i < (int) strlen(str); i++) {
		if (rs_buf_push(&buf, str[i]) == NULL) {
			rs_fatal("could not push to buffer:");
		}
	}
	const char *bufstr = rs_buf_cstr(&buf);

	// Make sure the returned string is nul-terminated.
	assert(bufstr[strlen(str)+1] == '\0');

	// Make sure the returned string matches the original string.
	assert(strcmp(bufstr, str) == 0);

	rs_buf_reset(&buf);
	TRACE("passed");
}
