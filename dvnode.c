#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "func.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <signal.h>

void cleanExit(){exit(0);}

static void die(char *x){perror(x);exit(1);}

int main(int argc, char **argv){
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
//clean up memory on exiting with ctrl+C
	char timestamp[25];
	int flag = 0;
	struct List neighbour;
	iniList(&neighbour);
	int nn = argc/2-1;
	unsigned short myport = atoi(argv[1]);
	struct List staticneighbour;
	iniList(&staticneighbour);
	for(int i = 0; i<nn; i++){
		loadnode(&neighbour,argv[2+i*2],argv[3+i*2],myport);
		loadnode(&staticneighbour,argv[2+i*2],argv[3+i*2],myport);
	}

	printtime(timestamp);
	fprintf(stderr,"%s ",timestamp);
	printlist(&staticneighbour);
//load input information into list of neighbour
	if(!strcmp(argv[argc-1], "last")){
		flag = 1;
	}
//if last is in input, set send flag on
	int recvskt = socket(AF_INET, SOCK_DGRAM, 0);
	if(recvskt<0){die("recvsocket failed.");}
	int sendskt = socket(AF_INET, SOCK_DGRAM, 0);
	if(sendskt<0){die("sendsocket failed.");}
//create listskt and sendskt
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=1;
	setsockopt(recvskt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	struct sockaddr_in myaddr;
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	myaddr.sin_port=htons(myport);
	if(bind(recvskt, (struct sockaddr *)&myaddr, sizeof(myaddr))<0){die("recvskt bind failed.");}
//bind port with recvskt
	char buffer[4096];
	int num;
	struct Node *node;
	int len = sizeof(struct info);
	struct sockaddr_in sendaddr;
	memset(&sendaddr,0,sizeof(sendaddr));
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	char dv[3];
	struct List recvinfo;
	iniList(&recvinfo);
	unsigned short nport;
	double x;
	struct Node *find;
	int first = 0;

//space reserved for variables

	while(1){
		if(flag == 1||first == 1){
			strncpy(buffer, "DV", 3);
			num = countlist(&neighbour);
			memcpy(buffer+3, &num, sizeof(int));
			node = neighbour.head;
			int c = 0;
			while(node){
				memcpy(buffer+7+c*len, node, len);
				node = node->next;
				c++;
			}	
			node = staticneighbour.head;
			while(node){
				sendaddr.sin_port = htons(node->to);
				sendto(sendskt, buffer, 7+len*num, 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr));
				printtime(timestamp);
				fprintf(stderr,"%s Message send from Node %d to Node %d\n",timestamp, myport, node->to);
				node = node->next;
			}				
			flag = 0;
			first=2;
		}

		if((recv(recvskt, buffer, sizeof(buffer), 0))>0){
			strncpy(dv, buffer, 3);
			if(!strcmp(dv, "DV")){
				if(first==0){first=1;}
				memcpy(&num, buffer+3, sizeof(int));
				for(int i = 0; i<num; i++){
					copyNode(&recvinfo, buffer+7+i*len);
				}
				node = recvinfo.head;
				nport = node->from;
				//printlist(&recvinfo);

				node = findonto(&staticneighbour, nport);
				x = node->rate;
				addlist(&recvinfo, x);
				//printlist(&recvinfo);
				printtime(timestamp);
				fprintf(stderr,"%s Message received at Node %d from Node %d\n",timestamp,myport,nport);
				//add the rate to neighbour to the recvinfo list
				node = recvinfo.head;
				while(node){
					if(node->to != myport){
						find = findonto(&neighbour, node->to);
						if(!find){
						//didn't find the node in neighbour
							addnode(&neighbour, node, myport);
							flag = 1;
						}else if((find->rate)>(node->rate)){
							find->rate = node->rate;
							find->hop = node->from;
							flag = 1;
						}
					}
					node = node->next;
				}
				printtime(timestamp);
				fprintf(stderr,"%s ",timestamp);
				printlist(&neighbour);
				removeAllNodes(&recvinfo);
			}
		}

	}
}
