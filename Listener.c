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

#include "packets.h"

#define BROAD_MAC0	0xFF
#define BROAD_MAC1	0xFF
#define BROAD_MAC2	0xFF
#define BROAD_MAC3	0xFF
#define BROAD_MAC4	0xFF
#define BROAD_MAC5	0xFF

#define MY_DEST_MAC0	0xFF
#define MY_DEST_MAC1	0xFF
#define MY_DEST_MAC2	0xFF
#define MY_DEST_MAC3	0xFF
#define MY_DEST_MAC4	0xFF
#define MY_DEST_MAC5	0xFF

#define MY_NAME "firat"
#define MY_SURNAME "sezel"

#define ETHER_TYPE	0x1234

#define DEFAULT_IF	"eth0"
#define BUF_SIZ		1024

//pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int length();//returns length of list
static void printList();//prints list
static void insert(char mac[6], char *name, char *surname);//inserts element to end of the list
static int findNormal(char mac[6]);//finding given hash value via comparing all values from list start to end
static void fill_query_bcast(struct query_bcast *q);
static void fill_chat(struct chat *q, char *message);
void sendChatMessage(char *message, char *name, char *surname, int type);
void sendChatACK(char mac[6], uint8_t id);
static void getMAC();

static uint8_t bcast[21];
static uint8_t ucast[41];
static uint8_t hcast[41];
static uint8_t ccast[54];
static uint8_t cacast[2];
static uint8_t ecast[21];

char *netInterface;
char mac1[6];
char mac2[6];
char AddressMac[6];
char ChatAckMac[6];
uint8_t MyMacAddress[6];

int sockfd, sockfa, sockfc, sockfack, sockmac;
uint8_t packageid = 1;

//linked list structure
struct node {
   char macAd[6];
   char *name;
   char *surname;
   unsigned long hash;
   struct node *next;
};

struct node *head = NULL;

/*get mac address of this computer*/
static void getMAC(){
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	int count;
	
	if(strcmp(netInterface, "none") == 0){
		strcpy(ifName, DEFAULT_IF);
	}else{
		strcpy(ifName, netInterface);
	}
	

	if ((sockmac = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockmac, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockmac, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	
	memset(sendbuf, 0, BUF_SIZ);
	
	MyMacAddress[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	MyMacAddress[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	MyMacAddress[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	MyMacAddress[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	MyMacAddress[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	MyMacAddress[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
}

//display the list - Listing includes chaining
static void printList() {
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
static int delete(char *name , char *surname){

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
static void insert(char mac[6], char *name, char *surname) {
    
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
static int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }

   return length;
}

/*finding value from sorted linked list starting from the 
beginning of the list to the end of the list*/
static int findNormal(char mac[6]){
	
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

static int findName(char *name, char *surname){
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

static int findMac(char *name, char *surname){
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

static int insertMac(char macad[6], char *name, char *surname){

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

static void fill_query_bcast(struct query_bcast *q)
{
    q->type = QUERY_BROADCAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

static void fill_query_ucast(struct query_ucast *q, char *tname, char *tsurname)
{
    q->type = QUERY_UNICAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
    snprintf(q->target_name, MAX_NAME_SIZE, "%s", tname);
    snprintf(q->target_surname, MAX_NAME_SIZE, "%s", tsurname);
}

static void fill_hello_response(struct hello_response *q, char *name, char *surname)
{
    q->type = HELLO_RESPONSE;
    snprintf(q->name, MAX_NAME_SIZE, "%s", name);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", surname);
    snprintf(q->target_name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->target_surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

static void fill_chat(struct chat *q, char *message)
{
    	q->type = CHAT;
    	q->length = (int)sizeof(message);
	
    	q->packed_id = packageid;
    	snprintf(q->message, MAX_MESSAGE_SIZE, "%s", strdup(message));
	packageid += 1;
	if(packageid == 255){
		packageid = 1;
	}
}

static void fill_chatack(struct chat_ack *q, uint8_t package_id){
	q->type = CHAT_ACK;
	q->packed_id = package_id;
}

static void fill_exiting(struct exiting *q){
	q->type = EXITING;
    	snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    	snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

static void decode_bcast(){

	struct query_bcast *q;
	q = (struct query_bcast*) bcast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->name: %s \n", q->name);
	fprintf(stdout, "q->surname: %s \n", q->surname);
	
	int a = insertMac(mac2, q->name, q->surname);
	
}

static void decode_ucast(char macad[6]){

	struct query_ucast *q;
	q = (struct query_ucast*) ucast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->name: %s \n", q->name);
	fprintf(stdout, "q->surname: %s \n", q->surname);
	fprintf(stdout, "q->target name: %s \n", q->target_name);
	fprintf(stdout, "q->target surname: %s \n", q->target_surname);

	if((strcmp(q->target_name, MY_NAME) == 0) &&  (strcmp(q->target_surname, MY_SURNAME) == 0) ){
		if(findName(q->name, q->surname) == -1){
			insertMac(macad, q->name, q->surname);
		}
		sendChatMessage("", q->name, q->surname, 1);//hello response
	}
	
}

static void decode_helres(){

	struct hello_response *q;
	q = (struct hello_response*) hcast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->responder name: %s \n", q->name);
	fprintf(stdout, "q->responder surname: %s \n", q->surname);
	fprintf(stdout, "q->queryier name: %s \n", q->target_name);
	fprintf(stdout, "q->queryier surname: %s \n", q->target_surname);

	if((strcmp(q->target_name, MY_NAME) == 0) &&  (strcmp(q->target_surname, MY_SURNAME) == 0) ){
		printf(" %s %s is here \n", q->target_name, q->target_surname);
	}
}

static void decode_chat(){

	struct chat *q;
	q = (struct chat*) ccast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->length: %d \n", q->length);
	fprintf(stdout, "q->packed id: %d \n", q->packed_id);
	fprintf(stdout, "q->mesage: %s \n", q->message);
	
}

static void decode_chatack(){

	struct chat_ack *q;
	q = (struct chat_ack*) cacast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->packed id: %d \n", q->packed_id);
	
}

static void decode_exiting(){

	struct exiting *q;
	q = (struct exiting*) ecast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->name: %s \n", q->name);
	fprintf(stdout, "q->surname: %s \n", q->surname);

	if(delete(q->name, q->surname) == 1){
		printf("%s %s leaves.", q->name, q->surname);
	}
	
}

void *Listen(void *vargp)
{
        char sender[INET6_ADDRSTRLEN];
	int ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	if(strcmp(netInterface, "none") == 0){
		strcpy(ifName, DEFAULT_IF);
	}else{
		strcpy(ifName, netInterface);
	}

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;

	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");	
		exit(-1);
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

repeat:	numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	/* Check the packet is for me */
	if (eh->ether_dhost[0] == MyMacAddress[0] &&
			eh->ether_dhost[1] == MyMacAddress[1] &&
			eh->ether_dhost[2] == MyMacAddress[2] &&
			eh->ether_dhost[3] == MyMacAddress[3] &&
			eh->ether_dhost[4] == MyMacAddress[4] &&
			eh->ether_dhost[5] == MyMacAddress[5]) {
		printf("PAKET BANA \n");
	}else if(eh->ether_dhost[0] == BROAD_MAC0 &&
			eh->ether_dhost[1] == BROAD_MAC1 &&
			eh->ether_dhost[2] == BROAD_MAC2 &&
			eh->ether_dhost[3] == BROAD_MAC3 &&
			eh->ether_dhost[4] == BROAD_MAC4 &&
			eh->ether_dhost[5] == BROAD_MAC5){
		printf("PAKET HERKESE \n");
	} else {
		ret = -1;
		goto done;
	}

	/* Look up my device IP addr if possible */
	strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFADDR, &if_ip) >= 0) { /* if we can't check then don't */
		/* ignore if I sent it */
		if (strcmp(sender, inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) == 0)	{
			printf("but I sent it :(\n");
			ret = -1;
			goto done;
		}
	}

	char *hex, *str;
	char type[3];
	uint8_t package_id;
	int m = 0;
	int sizeofdata;
	int s = 0;
	int macCount1 = 0;
	int macCount2 = 0;
	type[2] = buf[14];
		if(type[2] == 0x00){
			sizeofdata = 21;	
		}else if(type[2] == 0x01){
			sizeofdata= 41;
		}else if(type[2] == 0x02){
			sizeofdata= 41;
		}else if(type[2] == 0x03){
			sizeofdata= 54;
		}else if(type[2] == 0x04){
			sizeofdata= 2;
		}else if(type[2] == 0x05){
			sizeofdata= 21;
		}
	for (i=0; i<numbytes; i++){
		if(i >= 0 && i <=5){
			mac1[macCount1] = buf[i];
			macCount1++;		
		}
		else if(i >= 6 && i <=11){
			mac2[macCount2] = buf[i];
			macCount2++;
		}
		else if(i >= 12 && i <= 13){
			type[m] = buf[i];
			m++;
		}

		if(i >= 14 && i < 14 + sizeofdata){
			if(type[2] == 0x00){
				bcast[s] = buf[i];		
			}else if(type[2] == 0x01){
				ucast[s] = buf[i];
			}else if(type[2] == 0x02){
				hcast[s] = buf[i];
			}else if(type[2] == 0x03){
				ccast[s] = buf[i];
				package_id = buf[17];
			}else if(type[2] == 0x04){
				cacast[s] = buf[i];
			}else{
				ecast[s] = buf[i];
			}	
			s++;
		}	
	} 
	printf("DATA %02x\n", buf[14]);
	if(buf[14] == 0x00){
		decode_bcast();			
	}else if(buf[14] == 0x01){
		decode_ucast(mac2);
	}else if(buf[14] == 0x02){
		decode_helres();
	}else if(buf[14] == 0x03){
		printf("Message Received");
		decode_chat();
		sendChatACK(mac2, package_id);
	}else if(buf[14] == 0x04){
		decode_chatack();
		printf("Message Received");
	}else if(type[2] == 0x05){
		decode_exiting();
	}

	/*for(s = 0; s < 5; s++){
		printf("%c ", data[s]);
	}*/

	
	printf("\n");

done:	goto repeat;

	close(sockfd);
    return NULL;
}

//broadcast sending
void sender(int type){

	struct query_bcast query_bcast;
    	memset(&query_bcast, 0, sizeof(struct query_bcast));

	struct exiting ex;
	memset(&ex, 0, sizeof(struct exiting));

	if(type == 0){
		fill_query_bcast(&query_bcast);
	}else{
		fill_exiting(&ex);
	}

    	//hex_print((void *) &query_bcast, sizeof(struct query_bcast));	

	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	int count;
	
	/* Get interface name */
	if(strcmp(netInterface, "none") == 0){
		strcpy(ifName, DEFAULT_IF);
	}else{
		strcpy(ifName, netInterface);
	}

	/* Open RAW socket to send on */
	if ((sockfa = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfa, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	/* Ethernet header */
	eh->ether_shost[0] = MyMacAddress[0];
	eh->ether_shost[1] = MyMacAddress[1];
	eh->ether_shost[2] = MyMacAddress[2];
	eh->ether_shost[3] = MyMacAddress[3];
	eh->ether_shost[4] = MyMacAddress[4];
	eh->ether_shost[5] = MyMacAddress[5];
	eh->ether_dhost[0] = BROAD_MAC0;
	eh->ether_dhost[1] = BROAD_MAC1;
	eh->ether_dhost[2] = BROAD_MAC2;
	eh->ether_dhost[3] = BROAD_MAC3;
	eh->ether_dhost[4] = BROAD_MAC4;
	eh->ether_dhost[5] = BROAD_MAC5;
	/* Ethertype field */
	eh->ether_type = htons(ETHER_TYPE);
	tx_len += sizeof(struct ether_header);

	/* Packet data */
	
	if(type == 0){
		char* ptr= (char*)&query_bcast;
		for(count = 0; count < sizeof(query_bcast); count++){
			sendbuf[tx_len++] = *ptr;
			ptr++;
		}
	}else{
		char* ptr= (char*)&ex;
		for(count = 0; count < sizeof(ex); count++){
			sendbuf[tx_len++] = *ptr;
			ptr++;
		}
	}
	
	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = MY_DEST_MAC0;
	socket_address.sll_addr[1] = MY_DEST_MAC1;
	socket_address.sll_addr[2] = MY_DEST_MAC2;
	socket_address.sll_addr[3] = MY_DEST_MAC3;
	socket_address.sll_addr[4] = MY_DEST_MAC4;
	socket_address.sll_addr[5] = MY_DEST_MAC5;

	if (sendto(sockfa, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

	close(sockfa);
	
}

void sendChatACK(char mac[6], uint8_t id){
	struct chat_ack chack;

	memset(&chack, 0, sizeof(struct chat_ack));

    	fill_chatack(&chack, id);

	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	if(strcmp(netInterface, "none") == 0){
		strcpy(ifName, DEFAULT_IF);
	}else{
		strcpy(ifName, netInterface);
	}

	/* Open RAW socket to send on */
	if ((sockfack = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1)     {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfack, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");

	memset(sendbuf, 0, BUF_SIZ);
		/* Ethernet header */
	eh->ether_shost[0] = MyMacAddress[0];
	eh->ether_shost[1] = MyMacAddress[1];
	eh->ether_shost[2] = MyMacAddress[2];
	eh->ether_shost[3] = MyMacAddress[3];
	eh->ether_shost[4] = MyMacAddress[4];
	eh->ether_shost[5] = MyMacAddress[5];
	eh->ether_dhost[0] = mac[0];
	eh->ether_dhost[1] = mac[1];
	eh->ether_dhost[2] = mac[2];
	eh->ether_dhost[3] = mac[3];
	eh->ether_dhost[4] = mac[4];
	eh->ether_dhost[5] = mac[5];

	/* Ethertype field */
	eh->ether_type = htons(ETHER_TYPE);
	tx_len += sizeof(struct ether_header);

	/* Packet data */
	int count;
	char* ptr= (char*)&chack;
	for(count = 0; count < sizeof(chack); count++){
		sendbuf[tx_len++] = *ptr;
		ptr++;
	}

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = mac[0];
	socket_address.sll_addr[1] = mac[1];
	socket_address.sll_addr[2] = mac[2];
	socket_address.sll_addr[3] = mac[3];
	socket_address.sll_addr[4] = mac[4];
	socket_address.sll_addr[5] = mac[5];

	if (sendto(sockfack, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

	close(sockfack);
}

void sendChatMessage(char *message, char *name, char *surname, int type){ 
	struct chat ch;
	struct hello_response hr;
	struct query_ucast qu;

	if(type == 0){//chat message
		memset(&ch, 0, sizeof(struct chat));
		fill_chat(&ch, message);
	}else if(type == 1){//hello response
		memset(&hr, 0, sizeof(struct hello_response));
		fill_hello_response(&hr, name, surname);	
	}else{//query unicast
		memset(&qu, 0, sizeof(struct query_ucast));
		fill_query_ucast(&qu, name, surname);	
	}

	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	if(strcmp(netInterface, "none") == 0){
		strcpy(ifName, DEFAULT_IF);
	}else{
		strcpy(ifName, netInterface);
	}

	/* Open RAW socket to send on */
	if ((sockfc = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfc, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfc, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	if(findName(name, surname) != -1){
		/* Construct the Ethernet header */
		memset(sendbuf, 0, BUF_SIZ);
		/* Ethernet header */
		eh->ether_shost[0] = MyMacAddress[0];
		eh->ether_shost[1] = MyMacAddress[1];
		eh->ether_shost[2] = MyMacAddress[2];
		eh->ether_shost[3] = MyMacAddress[3];
		eh->ether_shost[4] = MyMacAddress[4];
		eh->ether_shost[5] = MyMacAddress[5];
		if(findMac(name, surname) == 1){
			printf("Bulduu");
			eh->ether_dhost[0] = AddressMac[0];
			eh->ether_dhost[1] = AddressMac[1];
			eh->ether_dhost[2] = AddressMac[2];
			eh->ether_dhost[3] = AddressMac[3];
			eh->ether_dhost[4] = AddressMac[4];
			eh->ether_dhost[5] = AddressMac[5];

			/* Ethertype field */
			eh->ether_type = htons(ETHER_TYPE);
			tx_len += sizeof(struct ether_header);

			/* Packet data */
			int count;
			if(type == 0){
				char* ptr= (char*)&ch;
				for(count = 0; count < sizeof(ch); count++){
					sendbuf[tx_len++] = *ptr;
					ptr++;
				}
			}else if(type == 1){
				char* ptr= (char*)&hr;
				for(count = 0; count < sizeof(hr); count++){
					sendbuf[tx_len++] = *ptr;
					ptr++;
				}
			}else{
				char* ptr= (char*)&qu;
				for(count = 0; count < sizeof(qu); count++){
					sendbuf[tx_len++] = *ptr;
					ptr++;
				}			
			}
			
			/* Index of the network device */
			socket_address.sll_ifindex = if_idx.ifr_ifindex;
			/* Address length*/
			socket_address.sll_halen = ETH_ALEN;
			/* Destination MAC */
			socket_address.sll_addr[0] = AddressMac[0];
			socket_address.sll_addr[1] = AddressMac[1];
			socket_address.sll_addr[2] = AddressMac[2];
			socket_address.sll_addr[3] = AddressMac[3];
			socket_address.sll_addr[4] = AddressMac[4];
			socket_address.sll_addr[5] = AddressMac[5];

			if (sendto(sockfc, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
			    printf("Send failed\n");
		}else{
			printf("Person Offlinee");
		}
	}else{
		printf("Person Offline");
			
	}

	close(sockfc);
}

void *Send(void *vargp){
	char name[10], surname[10], action[10], message[50];
	while(1){

		printf("Enter Action: ");
   		scanf("%s", action);
	
		if(strcmp(action, "chat") == 0){
			printf("Enter name: ");
   			scanf("%s", name);

			printf("Enter surname: ");
   			scanf("%s", surname);

			printf("Message: ");
   			scanf("%s", message);

			sendChatMessage(message, name, surname, 0);
		}else if(strcmp(action, "exit") == 0){
			sender(1);
			exit(0);
		}else if(strcmp(action, "online") == 0){
			printList();
		}else if(strcmp(action, "response") == 0){
			printf("Enter name: ");
   			scanf("%s", name);

			printf("Enter surname: ");
   			scanf("%s", surname);

			sendChatMessage("", name, surname, 2);//query unicast
		}	
	}
}

void *SendMyName(void *vargp){
	sleep(5);
	sender(0);
}

int main(int argc, char *argv[])
{
	//set network interface
	if(argv[1] != NULL){
		netInterface = argv[1];
	}else{
		netInterface = "none";
	}
	
	//get mac address of this computer as a source
	getMAC();
	//send first query_broadcast message to everyone
	sender(0);

	pthread_t tid;
	pthread_t tid2;
	//pthread_t tid3;
	
	pthread_create(&tid, NULL, Listen, NULL);//listen function
	pthread_create(&tid2, NULL, Send, NULL);//ui function
	//pthread_create(&tid3, NULL, SendMyName, NULL);//send query_broadcast in every 5 seconds

	pthread_join(tid2, NULL);
	pthread_join(tid, NULL);
	//pthread_join(tid3, NULL);

	return 0;
}
