#ifndef qMAIN_H
#define qMAIN_H
void send_msg(GIOChannel *h, uint32_t net, const char *name, const char *msg);
void temp_hide(GIOChannel *h, uint32_t net, const char *name);
void perm_hide(GIOChannel *h, uint32_t net, const char *name);

//notifier.c
void check_hilight(const char *buffer, const char* nick, const char* msg);
//minbif.c
void minbif_rename(GIOChannel *h, int type, int net, char *nick, char *msg);
//lua.c
void lua_message(GIOChannel *h, int type, int net, char *nick, char *msg);
void lua_timeout(GIOChannel *h);

#endif
