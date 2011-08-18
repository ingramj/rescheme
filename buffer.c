#include "rescheme.h"

#include <assert.h>

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
