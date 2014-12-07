This project aims at transforming your quassel core to a bot.
It currently has the following feature:
-Rename Gtalk and Facebook contacts in minbif
-Send notifications to IRSSI Notifier android application.
-Execute lua script config.lua

Copy qconfig.h.ex to qconfig.h and edit it to change what's built-in,
and to setup plugins

Edit notifier.c to control what to hilight on.

config.lua must define a msg(type, net, channel, nick, msg) callback.
It can use the send(net, channel, cmd), which sends a command, not a message.
Commands which begin without a / are considered as messages by Quassel.
config.lua can also define a timeout(pos) callback, which is called every second.

Support channel: #quassel-irssi @ irc.freenode.net
