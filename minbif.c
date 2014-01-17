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

#define _GNU_SOURCE
#include <asm/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iconv.h>
#include <ctype.h>
#include "quasselc.h"
#include "export.h"

#include "qconfig.h"
#include "main.h"

#ifdef MINBIF_RENAMER
char *stripname(char *str) {
	char *res = malloc(strlen(str));
	char *tmp = res;

	while(str[0]) {
		if(isalnum(*str)) {
			*tmp=*str;
			tmp++;
		}
		str++;
	}
	*tmp = 0;
	return res;

}

static void whois_handle(GIOChannel *h, int net, char *nick, char *_msg) {
#define FullName "Full Name:"
	if(!strstr(_msg, FullName))
		return;

	char *msg = strdup(_msg);
	char *fullname = strstr(msg, FullName);
	if(fullname == NULL)
		goto end;
	fullname[0]=0;
	fullname += sizeof(FullName);

#define Whois "[Whois]"
	char *name = strstr(msg, Whois);
	if(name == NULL)
		goto end;
	name += sizeof(Whois);

	char *stripped = stripname(fullname);
	printf("Renaming %s to %s (name=%s)\n", name, stripped, fullname);

	char *cmd = NULL;
	asprintf(&cmd, "/svsnick %s %s", name, stripped);
	printf("Sending %s\n", cmd);
	send_msg(h, net, "*", cmd);

	free(stripped);
	free(cmd);
end:
	free(msg);
}

static void join_handle(GIOChannel *h, int net, char *nick, char *msg) {
	if(isdigit(nick[1]) && (nick[0]=='-' || nick[0]=='_')) {
		char *cmd = NULL;
		asprintf(&cmd, "/whois %s %s", nick, nick);
		send_msg(h, net, "*", cmd);
		free(cmd);
	}
}

void minbif_rename(GIOChannel *h, int type, int net, char *nick, char *msg) {
	if(type)
		whois_handle(h, net, nick, msg);
	else
		join_handle(h, net, nick, msg);
}

#endif /* MINBIF_RENAMER */
