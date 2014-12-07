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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef LUA_BIND
static lua_State* L;
static GIOChannel *_h =NULL;
static int Lsend(lua_State *L) {
	int net = lua_tonumber(L, -3);
	const char *buffer = lua_tostring(L, -2);
	const char *cmd = lua_tostring(L, -1);

	if(_h != NULL)
		send_msg(_h, net, buffer, cmd);
	return 0;
}

static int Lperm_hide(lua_State *L) {
	int net = lua_tonumber(L, -2);
	const char *buffer = lua_tostring(L, -1);

	if(_h != NULL)
		perm_hide(_h, net, buffer);
	return 0;
}

static int Ltemp_hide(lua_State *L) {
	int net = lua_tonumber(L, -2);
	const char *buffer = lua_tostring(L, -1);

	if(_h != NULL)
		temp_hide(_h, net, buffer);
	return 0;
}

static int Lappend_buffer(lua_State *L) {
	int net = lua_tonumber(L, -2);
	const char *buffer = lua_tostring(L, -1);

	if(_h != NULL)
		append_buffer(_h, net, buffer);
	return 0;
}

static void __init(void) __attribute__((constructor));
static void __init(void) {
	L = luaL_newstate();
	luaL_openlibs(L);
	if(luaL_dofile(L, "config.lua")) {
		fprintf(stderr, "Got lua error: %s\n", lua_tostring(L, -1));
		exit(1);
	}
	lua_pushcfunction(L, Lsend);
	lua_setglobal(L, "send");

	lua_pushcfunction(L, Lperm_hide);
	lua_setglobal(L, "perm_hide");

	lua_pushcfunction(L, Ltemp_hide);
	lua_setglobal(L, "temp_hide");

	lua_pushcfunction(L, Lappend_buffer);
	lua_setglobal(L, "append_buffer");
}

void lua_msg(GIOChannel *h, int type, int net, char *buffer, char *nick, char *msg) {
	_h = h;
	lua_getglobal(L, "msg");
	if(lua_isnil(L, -1))
		return;
	if(lua_type(L, -1) != LUA_TFUNCTION) {
		fprintf(stderr, "msg must be a function\n");
		exit(1);
	}
	lua_pushnumber(L, type);
	lua_pushnumber(L, net);
	lua_pushstring(L, buffer);
	lua_pushstring(L, nick);
	lua_pushstring(L, msg);
	if(lua_pcall(L, 5, 0, 0)) {
		fprintf(stderr, "Got lua error: %s\n", lua_tostring(L, -1));
		exit(1);
	}
}

void lua_timeout(GIOChannel *h) {
	static int current = 0;
	_h = h;
	lua_getglobal(L, "timeout");
	if(lua_isnil(L, -1))
		return;
	if(lua_type(L, -1) != LUA_TFUNCTION) {
		fprintf(stderr, "timeout must be a function\n");
		exit(1);
	}
	lua_pushnumber(L, current);
	if(lua_pcall(L, 1, 0, 0)) {
		fprintf(stderr, "Got lua error: %s\n", lua_tostring(L, -1));
		exit(1);
	}
	current++;
}
#endif /* LUA_BIND */
