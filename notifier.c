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
#include "quasselc.h"
#include "export.h"

#include "qconfig.h"

#ifdef IRSSI_NOTIFIER
char* clean_string(char *buffer) {
	int i=0;
	while (buffer[i]) {
		if (buffer[i] == '\'') buffer[i] = '`';
		i++;
	}
	return buffer;
}


static void send_notif(const char *b, const char *s, const char *m) {
	char *buffer=clean_string(strdup(b));
	char *src=clean_string(strdup(s));
	char *msg=clean_string(strdup(m));

	char *cmd = NULL;
	asprintf(&cmd, "curl -d apiToken=" IRSSI_TOKEN
			" -d message=$(echo '%s'| openssl enc -aes-128-cbc -salt -base64 -A -pass pass:password|tr '+/' '-_' |sed -e 's/=//g') "
		       "-d nick=$(echo '%s' | openssl enc -aes-128-cbc -salt -base64 -A -pass pass:password |tr '+/' '-_' |sed -e 's/=//g' ) "
		       "-d channel=$(echo '%s' | openssl enc -aes-128-cbc -salt -base64 -A -pass pass:password |tr '+/' '-_' |sed -e 's/=//g') "
		       "-d version=18 https://irssinotifier.appspot.com/API/Message", msg, src, buffer);
	system(cmd);
	free(cmd);
	free(src);
	free(msg);
	free(buffer);
}

static int ischannel(const char *c) {
	if(*c== '#' || *c == '&')
		return 1;
	return 0;
}

void check_hilight(const char *buffer, const char* nick, const char* msg) {
	if(ischannel(buffer)) {
		if(strstr(msg, "phh"))
			send_notif(buffer, nick, msg);
	} else {
		send_notif(buffer, nick, msg);
	}
}

#endif /* IRSSI_NOTIFIER */
