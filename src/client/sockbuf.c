/*
 * BloodsPilot, a multiplayer space war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: sockbuf.c,v 1.3 2007/09/27 23:30:58 kps Exp $ */

#include "sysdeps.h"

#include "const.h"
#include "error.h"
#include "bit.h"
#include "sockbuf.h"
#include "socklib.h"


int last_packet_of_frame;

int Sockbuf_init(sockbuf_t * sbuf, sock_t * sock, int size, int state)
{
	if ((sbuf->buf = sbuf->ptr = (char *) malloc(size)) == NULL) {
		return -1;
	}
	if (sock != (sock_t *) NULL) {
		sbuf->sock = *sock;
	}
	else {
		sock_init(&sbuf->sock);
	}
	sbuf->state = state;
	sbuf->len = 0;
	sbuf->size = size;
	sbuf->ptr = sbuf->buf;
	sbuf->state = state;
	return 0;
}

int Sockbuf_cleanup(sockbuf_t * sbuf)
{
	if (sbuf->buf != NULL) {
		free(sbuf->buf);
	}
	sbuf->buf = sbuf->ptr = NULL;
	sbuf->size = sbuf->len = 0;
	sbuf->state = 0;
	return 0;
}

int Sockbuf_clear(sockbuf_t * sbuf)
{
	sbuf->len = 0;
	sbuf->ptr = sbuf->buf;
	return 0;
}

int Sockbuf_advance(sockbuf_t * sbuf, int len)
{
	/*
	 * First do a few buffer consistency checks.
	 */
	if (sbuf->ptr > sbuf->buf + sbuf->len) {
		errno = 0;
		error("Sockbuf pointer too far");
		sbuf->ptr = sbuf->buf + sbuf->len;
	}
	if (sbuf->ptr < sbuf->buf) {
		errno = 0;
		error("Sockbuf pointer bad");
		sbuf->ptr = sbuf->buf;
	}
	if (sbuf->len > sbuf->size) {
		errno = 0;
		error("Sockbuf len too far");
		sbuf->len = sbuf->size;
	}
	if (sbuf->len < 0) {
		errno = 0;
		error("Sockbuf len bad");
		sbuf->len = 0;
	}
	if (len <= 0) {
		if (len < 0) {
			errno = 0;
			error("Sockbuf advance negative (%d)", len);
		}
	}
	else if (len >= sbuf->len) {
		if (len > sbuf->len) {
			errno = 0;
			error("Sockbuf advancing too far");
		}
		sbuf->len = 0;
		sbuf->ptr = sbuf->buf;
	}
	else {
#if defined(__hpux) || defined(SVR4) || defined(_SEQUENT_) || defined(SYSV) || defined(_WINDOWS)
		memmove(sbuf->buf, sbuf->buf + len, sbuf->len - len);
#else
		bcopy(sbuf->buf + len, sbuf->buf, sbuf->len - len);
#endif
		sbuf->len -= len;
		if (sbuf->ptr - sbuf->buf <= len) {
			sbuf->ptr = sbuf->buf;
		}
		else {
			sbuf->ptr -= len;
		}
	}
	return 0;
}

int Sockbuf_flush(sockbuf_t * sbuf)
{
	int len, i;

	if (BIT(sbuf->state, SOCKBUF_WRITE) == 0) {
		errno = 0;
		error("No flush on non-writable socket buffer");
		error("(state=%02x,buf=%08x,ptr=%08x,size=%d,len=%d,sock=%d)",
		      sbuf->state, sbuf->buf, sbuf->ptr, sbuf->size, sbuf->len, sbuf->sock);
		return -1;
	}
	/*Trace("Sockbuf_flush: state=%02x,buf=%08x,ptr=%08x,size=%d,len=%d,sock=%d\n",
	   sbuf->state, sbuf->buf, sbuf->ptr, sbuf->size, sbuf->len,
	   sbuf->sock); */
	if (BIT(sbuf->state, SOCKBUF_LOCK) != 0) {
		errno = 0;
		error("No flush on locked socket buffer (0x%02x)", sbuf->state);
		return -1;
	}
	if (sbuf->len <= 0) {
		if (sbuf->len < 0) {
			errno = 0;
			error("Write socket buffer length negative");
			sbuf->len = 0;
			sbuf->ptr = sbuf->buf;
		}
		return 0;
	}

#if 0
	/* maintain a few statistics */
	{
		static int max = 1024, avg, count;

		avg += sbuf->len;
		count++;
		if (sbuf->len > max) {
			max = sbuf->len;
			printf("Max packet = %d, avg = %d\n", max, avg / count);
		}
		else if (max > 1024 && (count & 0x03) == 0) {
			max--;
		}
	}
#endif

	if (BIT(sbuf->state, SOCKBUF_DGRAM) != 0) {
		errno = 0;
		i = 0;
#if 0
		if (randomMT() % 12 == 0)	/* artificial packet loss */
			len = sbuf->len;
		else
#endif
			while ((len = sock_write(&sbuf->sock, sbuf->buf, sbuf->len)) <= 0) {
				if (len == 0 || errno == EWOULDBLOCK || errno == EAGAIN) {
					Sockbuf_clear(sbuf);
					return 0;
				}
				if (errno == EINTR) {
					errno = 0;
					continue;
				}
#if 0
				if (errno == ECONNREFUSED) {
					error("Send refused");
					Sockbuf_clear(sbuf);
					return -1;
				}
#endif
				if (++i > MAX_SOCKBUF_RETRIES) {
					error("Can't send on socket (%d,%d)", sbuf->sock,
					      sbuf->len);
					Sockbuf_clear(sbuf);
					return -1;
				}
				{
					static int send_err;
					if ((send_err++ & 0x3F) == 0) {
						error("send (%d)", i);
					}
				}
				if (sock_get_error(&sbuf->sock) == -1) {
					error("sock_get_error send");
					return -1;
				}
				errno = 0;
			}
		if (len != sbuf->len) {
			errno = 0;
			error("Can't write complete datagram (%d,%d)", len, sbuf->len);
		}
		Sockbuf_clear(sbuf);
	}
	else {
		errno = 0;
		while ((len = sock_write(&sbuf->sock, sbuf->buf, sbuf->len)) <= 0) {
			if (errno == EINTR) {
				errno = 0;
				continue;
			}
			if (errno != EWOULDBLOCK && errno != EAGAIN) {
				error("Can't write on socket");
				return -1;
			}
			return 0;
		}
		Sockbuf_advance(sbuf, len);
	}
	return len;
}

int Sockbuf_write(sockbuf_t * sbuf, char *buf, int len)
{
	if (BIT(sbuf->state, SOCKBUF_WRITE) == 0) {
		errno = 0;
		error("No write to non-writable socket buffer");
		return -1;
	}
	if (sbuf->size - sbuf->len < len) {
		if (BIT(sbuf->state, SOCKBUF_LOCK | SOCKBUF_DGRAM) != 0) {
			errno = 0;
			error("No write to locked socket buffer (%d,%d,%d,%d)",
			      sbuf->state, sbuf->size, sbuf->len, len);
			return -1;
		}
		if (Sockbuf_flush(sbuf) == -1) {
			return -1;
		}
		if (sbuf->size - sbuf->len < len) {
			return 0;
		}
	}
	memcpy(sbuf->buf + sbuf->len, buf, len);
	sbuf->len += len;

	return len;
}

int Sockbuf_read(sockbuf_t * sbuf)
{
	int max, i, len;

	if (BIT(sbuf->state, SOCKBUF_READ) == 0) {
		errno = 0;
		error("No read from non-readable socket buffer (%d)", sbuf->state);
		return -1;
	}
	if (BIT(sbuf->state, SOCKBUF_LOCK) != 0) {
		return 0;
	}
	if (sbuf->ptr > sbuf->buf) {
		Sockbuf_advance(sbuf, sbuf->ptr - sbuf->buf);
	}
	if ((max = sbuf->size - sbuf->len) <= 0) {
		static int before;
		if (before++ == 0) {
			errno = 0;
			error("Read socket buffer not big enough (%d,%d)", sbuf->size, sbuf->len);
		}
		return -1;
	}
	if (BIT(sbuf->state, SOCKBUF_DGRAM) != 0) {
		errno = 0;
		i = 0;
#if 0
		if (randomMT() % 12 == 0)	/* artificial packet loss */
			len = sbuf->len;
		else
#endif
			while ((len = sock_read(&sbuf->sock, sbuf->buf + sbuf->len, max)) <= 0) {
				if (len == 0) {
					return 0;
				}
#ifdef _WINDOWS
				errno = WSAGetLastError();
#endif
				if (errno == EINTR) {
					errno = 0;
					continue;
				}
				if (errno == EWOULDBLOCK || errno == EAGAIN) {
					return 0;
				}
#if 0
				if (errno == ECONNREFUSED) {
					error("Receive refused");
					return -1;
				}
#endif
/*
		Trace("errno=%d (%s) len = %d during sock_read\n", 
			errno, _GetWSockErrText(errno), len);
*/
				if (++i > MAX_SOCKBUF_RETRIES) {
					error("Can't recv on socket");
					return -1;
				}
				{
					static int recv_err;
					if ((recv_err++ & 0x3F) == 0) {
						error("recv (%d)", i);
					}
				}
				if (sock_get_error(&sbuf->sock) == -1) {
					error("GetSocketError recv");
					return -1;
				}
				errno = 0;
			}
		sbuf->len += len;
	}
	else {
		errno = 0;
		while ((len = sock_read(&sbuf->sock, sbuf->buf + sbuf->len, max)) <= 0) {
			if (len == 0) {
				return 0;
			}
			if (errno == EINTR) {
				errno = 0;
				continue;
			}
			if (errno != EWOULDBLOCK && errno != EAGAIN) {
				error("Can't read on socket");
				return -1;
			}
			return 0;
		}
/*	IFWINDOWS( Trace("Read stream %d bytes from %d\n", len, sbuf->sock); )*/
		sbuf->len += len;
	}

	return sbuf->len;
}

int Sockbuf_copy(sockbuf_t * dest, sockbuf_t * src, int len)
{
	if (len < dest->size - dest->len) {
		errno = 0;
		error("Not enough room in destination copy socket buffer");
		return -1;
	}
	if (len < src->len) {
		errno = 0;
		error("Not enough data in source copy socket buffer");
		return -1;
	}
	memcpy(dest->buf + dest->len, src->buf, len);
	dest->len += len;

	return len;
}
