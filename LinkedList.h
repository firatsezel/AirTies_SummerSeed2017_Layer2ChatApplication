#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

int length();//returns length of list
void printList();//prints list
void insert(char mac[6], char *name, char *surname);//inserts element to end of the list
int findNormal(char mac[6]);//finding given hash value via comparing all values from list start to end
unsigned long hash(char str[6]);
int delete(char *name , char *surname);
int findName(char *name, char *surname);
int findMac(char *name, char *surname);
int insertMac(char macad[6], char *name, char *surname);

char AddressMac[6];

#endif
