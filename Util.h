#ifndef _UTIL_H
#define _UTIL_H

#include "packets.h"

void fill_query_bcast(struct query_bcast *q);
void fill_query_ucast(struct query_ucast *q, char *tname, char *tsurname);
void fill_chat(struct chat *q, char *message);
void fill_hello_response(struct hello_response *q, char *name, char *surname);
void fill_chat(struct chat *q, char *message);
void fill_chatack(struct chat_ack *q, uint8_t package_id);
void fill_exiting(struct exiting *q);
void decode_bcast();
void decode_ucast(char macad[6]);
void decode_helres();
void decode_chat();
void decode_chatack();
void decode_exiting();
void getMAC();

uint8_t bcast[21];
uint8_t ucast[41];
uint8_t hcast[41];
uint8_t ccast[54];
uint8_t cacast[2];
uint8_t ecast[21];

char mac2[6];

uint8_t MyMacAddress[6];
char *netInterface;
int sockmac;
uint8_t packageid;

#endif
