//display the list - Listing includes chaining
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <math.h>
#include <fcntl.h> 
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "LinkedList.h" 

int length();//returns length of list
void printList();//prints list
void insert(char mac[6], char *name, char *surname);//inserts element to end of the list
int findNormal(char mac[6]);//finding given hash value via comparing all values from list start to end
unsigned long hash(char str[6]);
int delete(char *name , char *surname);
int findName(char *name, char *surname);
int findMac(char *name, char *surname);
int insertMac(char macad[6], char *name, char *surname);

//linked list structure
struct node {
   char macAd[6];
   char *name;
   char *surname;
   unsigned long hash;
   struct node *next;
};

struct node *head = NULL;

void printList() {
   struct node *ptr = head;
   struct node *chains;
   struct node *temp;
   
   printf("LIST : \n");
   //start from the beginning
   while(ptr != NULL) {
      printf("%s %s \n",ptr->name, ptr->surname);
      temp = ptr;
      ptr = ptr->next;
   }
}

unsigned long hash(char str[6])
{
    unsigned long hash = 5381;
    int c;
    int i;
    int endofsize = 0;
    for(i = 0; i < 6; i++){
	c = str[i];
	hash = (((hash << 5) + (hash * c))) + (c * 37);
    }
   
    return hash;
}

//delete function for linked list
int delete(char *name , char *surname){

    if(head != NULL){
	    struct node *temp = (struct node *)malloc(sizeof(struct node));
	    struct node *right =  (struct node *)malloc(sizeof(struct node));

	    struct node *current;
	    current = head;
	    for(current; current != NULL; current = current->next) {
		 if(current->next != NULL){
			right = current->next;
		 }else{
			if((strcmp(current->name, name) == 0) && (strcmp(current->surname, surname) == 0)){
				head = NULL;
				return 1;
		  	}
		 }
		 if((strcmp(right->name, name) == 0) && (strcmp(right->surname, surname) == 0)){
		 	if(right->next != NULL){
				current->next = right->next;	
				return 1;	
			}else{
				current->next = NULL;	
				return 1;;	
			}
		 }
	    }
    }else{
	return -1;
    }

    return -1;
}

//insert last location
void insert(char mac[6], char *name, char *surname) {
    
    struct node *temp =  (struct node *)malloc(sizeof(struct node));
    struct node *right =  (struct node *)malloc(sizeof(struct node));
    memcpy(temp->macAd, mac, 6);    
    temp->name = strdup(name);
    temp->surname = strdup(surname);
    temp->hash = hash(mac);
	
    if(head == NULL){
	head = temp;
	head->next = NULL;
    }else{
	right = head;
   
    	while(right->next != NULL){
    		right=right->next;
    	}
    	right->next = temp;
    	right=temp;
    	right->next=NULL;
    }
}

//returns length of list
int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }

   return length;
}

/*finding value from sorted linked list starting from the 
beginning of the list to the end of the list*/
int findNormal(char mac[6]){
	
   unsigned long hashval = hash(mac);
   struct node *current;
	int i = 0;
   for(current = head; current != NULL; current = current->next) {
        if(current->hash == hashval){
        	return i;
        }
        i++;
   }
   return -1;

}

int findName(char *name, char *surname){
   struct node *current;
   int i = 0;
   for(current = head; current != NULL; current = current->next) {
        if((strcmp(current->name, name) == 0) &&
(strcmp(current->surname, surname) == 0)){
		memcpy(AddressMac, current->macAd, 6);
        	return i;
        }
        i++;
   }
   return -1;
}

int findMac(char *name, char *surname){
   struct node *current;
   int i = 0;
   for(current = head; current != NULL; current = current->next) {
        if((strcmp(current->name, name) == 0) &&
(strcmp(current->surname, surname) == 0)){
		memcpy(current->macAd, AddressMac, 6);
        	return 1;
        }
        i++;
   }
   return -1;
}

int insertMac(char macad[6], char *name, char *surname){

	if(head == NULL){
		insert(macad, name, surname);	
		return 1;
	}else{
		//if(findNormal(macad) == -1){
			insert(macad, name, surname);	
			return 2;
		//}
	}
	
	return 0;
	
}
