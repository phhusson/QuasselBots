/*
   This file is part of QuasselC.

   QuasselC is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   QuasselC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with QuasselC.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <asm/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iconv.h>
#include "quasselc.h"
#include "qconfig.h"
#include "export.h"

#include "main.h"

#if 0
#define dprintf(x...) printf(x)
#else
static inline void useless_printf(char *str, ...) {
	(void) str;
}
#define dprintf useless_printf
#endif

struct buffer {
	struct bufferinfo i;
	int lastseen;
	int marker;
	int displayed;
};
static struct buffer *buffers;
static int n_buffers;

static int find_buffer_id(uint32_t net, char *name) {
	int i;
	for(i=0;i<n_buffers;++i) {
		if(buffers[i].i.id==(uint32_t)-1)
			continue;
		if(
			(strcmp(buffers[i].i.name, name)==0 || strcmp(name, "*")==0)
				&& (net==(uint32_t)-1 || net == buffers[i].i.network))
			return i;
	}
	return -1;
}

void send_msg(GIOChannel *h, uint32_t net, char *name, char *msg) {
	int id = find_buffer_id(net, name);
	if(id==-1)
		return;
	send_message(h, buffers[id].i, msg);
}

void handle_backlog(struct message m, void *arg) {
	(void) arg;
	(void) m;
}

void handle_message(struct message m, void *arg) {
	GIOChannel *chan = (GIOChannel*)arg;
	char *nick=strdup(m.sender);
	if(index(nick, '!'))
		*index(nick, '!')=0;
	printf("%s: %s says (type=%d) %s\n", m.buffer.name, nick, m.type, m.content);
#ifdef IRSSI_NOTIFIER
	if(m.type == 1)
		check_hilight(m.buffer.name, nick, m.content);
#endif

#ifdef MINBIF_RENAMER
	if(m.type == 1024 || m.type == 32)
		minbif_rename(chan, m.type == 1024, m.buffer.network, nick, m.content);
#endif
#ifdef LUA_BIND
	lua_msg(chan, m.type, m.buffer.network, m.buffer.name, nick, m.content);
#endif
	free(nick);
}

void handle_sync(void* arg, object_t o, function_t f, ...) {
	(void) arg;
	//Should be used to ensure f consistency
	(void) o;
	va_list ap;
	char *fnc=NULL;
	char *net,*chan,*nick,*name,*str;
	int netid,bufid,msgid,group,type;
	int latency,away;
	va_start(ap, f);
	switch(f) {
		/* BufferSyncer */
		case Create:
			bufid=va_arg(ap, int);
			netid=va_arg(ap, int);
			type=va_arg(ap, int);
			group=va_arg(ap, int);
			name=va_arg(ap, char*);
			dprintf("CreateBuffer(%d, %d, %s)\n", netid, bufid, name);
			if(bufid>=n_buffers) {
				buffers=realloc(buffers, sizeof(struct buffer)*(bufid+1));
				int i;
				for(i=n_buffers;i<=bufid;++i)
					buffers[i].i.id=-1;
				n_buffers=bufid+1;
			}
			buffers[bufid].i.network=netid;
			buffers[bufid].i.id=bufid;
			buffers[bufid].i.type=type;
			buffers[bufid].i.group=group;
			buffers[bufid].i.name=name;
			buffers[bufid].marker=0;
			buffers[bufid].lastseen=0;
			buffers[bufid].displayed=1;
			break;
		case MarkBufferAsRead:
			if(!fnc)
				fnc="MarkBufferAsRead";
		case Displayed:
			if(!fnc)
				fnc="BufferDisplayed";
			bufid=va_arg(ap, int);
			dprintf("%s(%d)\n", fnc, bufid);
			buffers[bufid].displayed=1;
			break;
		case Removed:
			if(!fnc)
				fnc="BufferRemoved";
		case TempRemoved:
			if(!fnc)
				fnc="BufferTempRemoved";
			bufid=va_arg(ap, int);
			buffers[bufid].displayed=0;
			dprintf("%s(%d)\n", fnc, bufid);
			break;
		case SetLastSeenMsg:
			if(!fnc)
				fnc="SetLastSeenMsg";
			bufid=va_arg(ap, int);
			msgid=va_arg(ap, int);
			buffers[bufid].lastseen=msgid;
			dprintf("%s(%d, %d)\n", fnc, bufid, msgid);
			break;
		case SetMarkerLine:
			if(!fnc)
				fnc="SetMarkerLine";
			bufid=va_arg(ap, int);
			msgid=va_arg(ap, int);
			buffers[bufid].marker=msgid;
			dprintf("%s(%d, %d)\n", fnc, bufid, msgid);
			break;
		/* IrcChannel */
		case JoinIrcUsers:
			net=va_arg(ap, char*);
			chan=va_arg(ap, char*);
			int size=va_arg(ap, int);
			char **users=va_arg(ap, char**);
			char **modes=va_arg(ap, char**);
			if(size==0)
				break;
			if(size>1) {
				printf("Too many users joined\n");
				break;
			}
			dprintf("JoinIrcUser(%s, %s, %s, %s)\n", net, chan, users[0], modes[0]);
			break;
		case AddUserMode:
			if(!fnc)
				fnc="AddUserMode";
		case RemoveUserMode:
			if(!fnc)
				fnc="RemoveUserMode";
			net=va_arg(ap, char*);
			chan=va_arg(ap, char*);
			nick=va_arg(ap, char*);
			char *mode=va_arg(ap, char*);
			dprintf("%s(%s, %s, %s, %s)\n", fnc, net, chan, nick, mode);
			break;
		/* IrcUser */
		case SetNick2:
			if(!fnc)
				fnc="SetNick";
		case Quit:
			if(!fnc)
				fnc="Quit";
			net=va_arg(ap, char*);
			nick=va_arg(ap, char*);
			dprintf("%s(%s, %s)\n", fnc, net, nick);
			break;
		case SetNick:
			if(!fnc)
				fnc="SetNick";
		case SetServer:
			if(!fnc)
				fnc="SetServer";
		case SetRealName:
			if(!fnc)
				fnc="SetRealName";
		case PartChannel:
			if(!fnc)
				fnc="PartChannel";
			net=va_arg(ap, char*);
			nick=va_arg(ap, char*);
			str=va_arg(ap, char*);
			dprintf("%s(%s, %s, %s)\n", fnc, net, nick, str);
			break;
		case SetAway:
			net=va_arg(ap, char*);
			nick=va_arg(ap, char*);
			away=va_arg(ap, int);
			dprintf("setAway(%s, %s, %s)\n", net, nick, away ? "away" : "present");
			break;
		/* Network */
		case AddIrcUser:
			net=va_arg(ap, char*);
			name=va_arg(ap, char*);
			dprintf("AddIrcUser(%s, %s)\n", net, name);
			break;
		case SetLatency:
			net=va_arg(ap, char*);
			latency=va_arg(ap, int);
			dprintf("SetLatency(%s, %d)\n", net, latency);
			break;
	}
	va_end(ap);
}

static char* user = NULL;
static char* pass = NULL;
void handle_event(void* arg, GIOChannel* h, event_t t, ...) {
	(void) arg;
	va_list ap;
	va_start(ap, t);
	switch(t) {
		case ClientInitAck:
			quassel_login(h, user, pass);
			break;
		case SessionInit:
			initRequest(h, "BufferViewStatus", "0");
			break;
		case TopicChange:
			break;
		case ChanPreAddUser:
			break;
		case ChanReady:
			break;
	}
	va_end(ap);
}

//Copy/pasted from irssi/network.c
static inline int read_io(GIOChannel *handle, char *buf, int len)
{
	gsize ret;
	GIOStatus status;
	GError *err = NULL;

	g_return_val_if_fail(handle != NULL, -1);
	g_return_val_if_fail(buf != NULL, -1);

	status = g_io_channel_read_chars(handle, buf, len, &ret, &err);
	if (err != NULL) {
		g_warning(err->message);
		g_error_free(err);
	}
	if (status == G_IO_STATUS_ERROR || status == G_IO_STATUS_EOF)
		return -1; /* disconnected */

	return ret;
}

typedef struct {
	char *msg;
	uint32_t size;
	uint32_t got;
} net_buf;

static int io_handler(GIOChannel *chan, GIOCondition condition, gpointer data) {
	(void) condition;
	net_buf *b = (net_buf*)data;
	if(!b->size) {
		uint32_t size;
		if(read_io(chan, (char*)&size, 4)!=4)
			return 1;
		size=htonl(size);
		if(size==0)
			return 1;
		b->msg = malloc(size);
		if(!b->msg)
			return 1;
		b->size = size;
		b->got = 0;
	}

	b->got += read_io(chan, b->msg+b->got, b->size-b->got);
	if(b->got == b->size) {
		quassel_parse_message(chan, b->msg, chan);
		//display_qvariant(b->msg);
		free(b->msg);
		b->got = 0;
		b->size = 0;
	}
	return 1;
}

static int create_socket(const char* host, const char* port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully bind(2).
	   If socket(2) (or bind(2)) fails, we (close the socket
	   and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Success */

		close(sfd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);
	return sfd;
}

int main(int argc, char **argv) {
	if(argc != 5) {
		fprintf(stderr, "%s: <host> <port> <user> <pass>\n", argv[0]);
		return 1;
	}

	net_buf b;
	b.msg = NULL;
	b.size = 0;
	b.got = 0;

	user = argv[3];
	pass = argv[4];
	GIOChannel *in = g_io_channel_unix_new(create_socket(argv[1], argv[2]));
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_buffered(in, FALSE);
	quassel_init_packet(in, 0);
	g_io_add_watch(in, G_IO_IN, io_handler, &b);
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
}
