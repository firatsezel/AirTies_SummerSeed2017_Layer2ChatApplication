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

#include "Util.h"
#include "LinkedList.h"
#include "packets.h"
#include "Listener.h"

#define MY_NAME "firat"
#define MY_SURNAME "sezel"

#define BUF_SIZ		1024

#define ETHER_TYPE	0x1234

#define DEFAULT_IF	"eth0"

/*Get Mac Address of this computer*/
void getMAC(){
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



void fill_query_bcast(struct query_bcast *q)
{
    q->type = QUERY_BROADCAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

void fill_query_ucast(struct query_ucast *q, char *tname, char *tsurname)
{
    q->type = QUERY_UNICAST;
    snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
    snprintf(q->target_name, MAX_NAME_SIZE, "%s", tname);
    snprintf(q->target_surname, MAX_NAME_SIZE, "%s", tsurname);
}

void fill_hello_response(struct hello_response *q, char *name, char *surname)
{
    q->type = HELLO_RESPONSE;
    snprintf(q->name, MAX_NAME_SIZE, "%s", name);
    snprintf(q->surname, MAX_NAME_SIZE, "%s", surname);
    snprintf(q->target_name, MAX_NAME_SIZE, "%s", MY_NAME);
    snprintf(q->target_surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

void fill_chat(struct chat *q, char *message)
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

void fill_chatack(struct chat_ack *q, uint8_t package_id){
	q->type = CHAT_ACK;
	q->packed_id = package_id;
}

void fill_exiting(struct exiting *q){
	q->type = EXITING;
    	snprintf(q->name, MAX_NAME_SIZE, "%s", MY_NAME);
    	snprintf(q->surname, MAX_NAME_SIZE, "%s", MY_SURNAME);
}

void decode_bcast(){

	struct query_bcast *q;
	q = (struct query_bcast*) bcast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->name: %s \n", q->name);
	fprintf(stdout, "q->surname: %s \n", q->surname);
	
	int a = insertMac(mac2, q->name, q->surname);
	
}

void decode_ucast(char macad[6]){

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

void decode_helres(){

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

void decode_chat(){

	struct chat *q;
	q = (struct chat*) ccast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->length: %d \n", q->length);
	fprintf(stdout, "q->packed id: %d \n", q->packed_id);
	fprintf(stdout, "q->mesage: %s \n", q->message);
	
}

void decode_chatack(){

	struct chat_ack *q;
	q = (struct chat_ack*) cacast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->packed id: %d \n", q->packed_id);
	
}

void decode_exiting(){

	struct exiting *q;
	q = (struct exiting*) ecast;

	fprintf(stdout, "q->type: %d \n", q->type);
	fprintf(stdout, "q->name: %s \n", q->name);
	fprintf(stdout, "q->surname: %s \n", q->surname);

	if(delete(q->name, q->surname) == 1){
		printf("%s %s leaves.", q->name, q->surname);
	}
	
}
