#include "rescheme.h"

#define _RS_BUF_GROWBY 128


void rs_buf_init(struct rs_buf *buf)
{
	if (buf != NULL) {
		buf->buf = NULL;
		buf->capacity = 0;
		buf->offset = 0;
	}
}


void rs_buf_reset(struct rs_buf *buf)
{
	if (buf != NULL) {
		free(buf->buf);
		buf->capacity = 0;
		buf->offset = 0;
	}
}


struct rs_buf *rs_buf_push(struct rs_buf *buf, char c)
{
	if (buf == NULL) {
		errno = EINVAL;
		return NULL;
	}

	if (buf->buf == NULL) {
		buf->buf = calloc(_RS_BUF_GROWBY, 1);
		if (buf->buf == NULL) {
			return NULL;
		}
		buf->capacity = _RS_BUF_GROWBY;
		buf->offset = 0;
	}

	if (buf->offset >= buf->capacity - 1) {
		char *newbuf = realloc(buf->buf, buf->capacity + _RS_BUF_GROWBY);
		if (newbuf == NULL) {
			return NULL;
		}
		buf->buf = newbuf;
		buf->capacity += _RS_BUF_GROWBY;
	}
	buf->buf[buf->offset++] = c;

	return buf;
}


const char *rs_buf_str(struct rs_buf *buf)
{
	buf->buf[buf->offset] = '\0';
	return (const char *) buf->buf;
}
