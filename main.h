#ifndef qMAIN_H
#define qMAIN_H
void send_msg(GIOChannel *h, uint32_t net, char *name, char *msg);

//notifier.c
void check_hilight(const char *buffer, const char* nick, const char* msg);
//minbif.c
void minbif_rename(GIOChannel *h, int type, int net, char *nick, char *msg);

#endif
