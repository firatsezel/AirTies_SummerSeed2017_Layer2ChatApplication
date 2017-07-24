#ifndef _LISTENER_H
#define _LISTENER_H

void sendChatMessage(char *message, char *name, char *surname, int type);
void sender(int type);
void sendChatACK(char mac[6], uint8_t id);

#endif
