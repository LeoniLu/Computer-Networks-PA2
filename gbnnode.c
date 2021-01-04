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


static void die(char *x){ perror(x); exit(1);}

int main(int argc, char **argv){
	unsigned short selfport, peerport;
	int N;
	double dropv;
	char mode[3];
	srand(time(NULL));
	int count = 0;
	int sendpkt;
	int senddrop;
	int recvpkt;
	int recvdrop;

	if(argc==6){
		selfport = atoi(argv[1]);
		peerport = atoi(argv[2]);
		N = atoi(argv[3]);	
		strcpy(mode,argv[4]);
		dropv = strtod(argv[5],NULL);
	}else{
		die("useage: ./gbnnode <selfport> <peerport> <windowsize> [optional -d/-p <value>]");
	}
	if(strcmp(mode,"-d") && strcmp(mode,"-p")){die("probability input incorrect.");}
//take in input arguments
	int skt = socket(AF_INET, SOCK_DGRAM,0);
	if(skt<0){die("socket failed.");}
//create socket
	struct sockaddr_in myaddr;
	memset(&myaddr,0,sizeof(myaddr));
	myaddr.sin_family=AF_INET;
	myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	myaddr.sin_port = htons(selfport);
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=1;
	setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	if(bind(skt, (struct sockaddr *)&myaddr, sizeof(myaddr))<0){die("bind failed.");}
//non-blocking socket with countdown time set to 0, bind socket with port
	char input[4096];
	char rec[4096];
	char flag[4];
	int seq=10000;
	int base=10000;//set both to non zero num in case it is not initialized
	struct packet pkt;
	int start=0;
	char sen[4096];
	char timestamp[25];
	char message[4096];
	int length;
	int st;
	int sseq=10000;
	int comseq;
	int o=1;
	char msg[4096];
	memset(&msg, 0,sizeof(msg));
	int recvf=0;


//place reserved for variable declaration
	struct sockaddr_in peeraddr_in;
	memset(&peeraddr_in, 0, sizeof(peeraddr_in));
	peeraddr_in.sin_family=AF_INET;
	peeraddr_in.sin_addr.s_addr=htonl(INADDR_ANY);
	peeraddr_in.sin_port = htons(peerport);
	struct sockaddr *peeraddr = (struct sockaddr *) &peeraddr_in;
	socklen_t len = sizeof(peeraddr_in);
//create sockaddr peeraddr for the peer
	fprintf(stderr,"node> ");


	while(1){
//detect send input arguments
		fd_set rfds;
		FD_ZERO(&rfds);
          	FD_SET(0, &rfds);
		if(select(1, &rfds, NULL, NULL, &timeout)){
//fprintf(stderr,"read stdin\n");//okay
			if(fgets(input, sizeof(input), stdin)){
				char *issend;
				issend = strtok(input," ");
				if(!strcmp(issend, "send")){
//fprintf(stderr,"send received\n");//okay
					strcpy(message, input+5);
					length = strlen(message);
					sseq=0;
					st=0;
					base = sseq;
					comseq=0;
					memset(sen, 0, sizeof(sen));
					strncpy(sen, "STA",4);
					memcpy(sen+4, &sseq, sizeof(sseq));
					memcpy(sen+8, &selfport, sizeof(short));
					while(1){
						if(sendto(skt, sen, 10, 0, peeraddr, len)<0){die("send failed.");}
//fprintf(stderr,"begin sequence sent\n");//okay
						recv(skt, rec, sizeof(rec), 0);
						strncpy(flag, rec, 4);
						memcpy(&comseq, rec+4, 4);
						if(comseq==sseq){break;}
					}//make sure the initialization of seq number is finished
					o=1;
//fprintf(stderr,"initialization finished\n");//okay

					while(o){
						clock_t d = 500+clock();
						while(d>clock()){
						if((sseq-base<=N-1)&&(sseq<st+length)){
							memset(sen, 0, sizeof(sen));
							strncpy(sen, "DAT",4);
							memcpy(sen+4, &sseq,4);
							memcpy(sen+8, &selfport, sizeof(short));
							memcpy(sen+10, message+sseq,1);
							sendto(skt, sen, 11, 0, peeraddr, len);
							if(sseq==base){d = 500+clock();}
							if(sseq!=st+length-1){
								printtime(timestamp);
								fprintf(stderr, "%s packet%d %c sent\n",timestamp, sseq, *(message+sseq));
							}else{
								printtime(timestamp);
								fprintf(stderr,"%s packet %d [\\n] sent \n",timestamp, sseq);
							}
							sseq++;
						}


						if((recv(skt, rec, sizeof(rec),0))>0){
							strncpy(flag, rec, 4);
							
							if(!strcmp(flag, "ACK")){
								memcpy(&comseq, rec+4, 4);
								if(decide(mode,dropv,&count)){
									sendpkt++;
									if(comseq>=base && comseq!=st+length-1){
										base=comseq+1;
										printtime(timestamp);
										fprintf(stderr, "%s ACK%d received, window moves to %d\n",timestamp, comseq, base);
									}else if(comseq==st+length-1){
										//last ack received, summary here
										printtime(timestamp);
										fprintf(stderr, "%s ACK%d received, all ACK received.\n", timestamp, comseq);
										fprintf(stderr,"[summary] %d/%d packets discarded, loss rate = %d%%\nnode> ",senddrop,sendpkt,(senddrop*100/sendpkt));
										senddrop=0;
										count=0;
										sendpkt=0;
										o=0;
									}else{
										printtime(timestamp);
										fprintf(stderr,"%s ACK%d discarded\n",timestamp, comseq);
									}
								}else{
									sendpkt++;
									senddrop++;
									printtime(timestamp);
									fprintf(stderr,"%s ACK%d discarded\n",timestamp,comseq);
									comseq=0;
								}


							}
						}//if recv
						}//while d>clock
						if(comseq!=st+length-1){
						printtime(timestamp);
						fprintf(stderr,"%s ACK%d timeout\n",timestamp, base);
						sseq=base;}
					}

				}// end if send
						
			}//end get stdin
		}//end select

//recv initialization of sequence number
		if((recv(skt, rec, sizeof(rec), 0))>0){
//fprintf(stderr,"****message received\n");
			strncpy(flag, rec,4);
			if(!strcmp(flag, "STA")){
//fprintf(stderr,"initial entered\n");
				memcpy(&seq, rec+4, sizeof(int));//initialize sequence record
				start = seq;//assign start to first seq
				memset(sen, 0, sizeof(sen));
				recvf=1;
				strcpy(sen, "ACK");
				memcpy(sen+4, &start, sizeof(int));
				memcpy(sen+8, &selfport, sizeof(short));
				sendto(skt, sen, 10, 0, peeraddr, len);//after initialization received, return ACK + start
//fprintf(stderr,"initial ACK send\n");
			}else if(!strcmp(flag, "DAT")&&recvf==1){
				memcpy(&pkt, rec, sizeof(pkt));
				if(decide(mode,dropv,&count)){
				recvpkt++;
				if(pkt.data!='\n'){
					printtime(timestamp);
					fprintf(stderr, "%s packet%d %c received\n", timestamp, pkt.seq, pkt.data);
					memset(sen,0,sizeof(sen));
					strncpy(sen, "ACK",4);
					memcpy(sen+8, &selfport, sizeof(short));
					//return ACK + seq + port
					if(pkt.seq==seq){
						strncpy(msg+seq-start,&(pkt.data),1);
						memcpy(sen+4, &seq, sizeof(int));
						printtime(timestamp);
						fprintf(stderr,"%s ACK%d sent, expecting packet %d\n",timestamp, seq, seq+1);
						seq++;
						sendto(skt, sen, 10, 0, peeraddr, len);
					}else{
						printtime(timestamp);
						fprintf(stderr,"%s packet%d %c discarded\n",timestamp, pkt.seq,pkt.data); 
						int now = seq-1;
						memcpy(sen+4, &now, sizeof(int));
						sendto(skt, sen, 10, 0, peeraddr, len);
						printtime(timestamp);
						fprintf(stderr,"%s ACK%d sent, expecting packet %d\n",timestamp, seq-1, seq);

					}
				}else{
				//if a '\n' is receivied
					//[summary here]
					if(pkt.seq==seq){
					memset(sen,0,sizeof(sen));
					strncpy(sen, "ACK",4);
					memcpy(sen+8, &selfport, sizeof(short));
					memcpy(sen+4,&(pkt.seq),sizeof(int));
					sendto(skt,sen,10,0,peeraddr,len);
					printtime(timestamp);
					fprintf(stderr, "%s ACK%d sent, last packet received.\n",timestamp, pkt.seq);
					clock_t delay = 2*N*500+clock();
					while(delay>clock()){
						if((recv(skt, rec, sizeof(rec),0))>0){
							strncpy(sen,"ACK",4);
							memcpy(sen+4, rec+4,sizeof(int));
							printtime(timestamp);
fprintf(stderr,"%s ACK%d resend in waiting to terminate period\n",timestamp, *((int *)(rec+4)));
							sendto(skt,sen,10,0,peeraddr,len);
						}
					}
					fprintf(stderr,"[summary] %d/%d packet dropped, loss rate = %d%%\n",recvdrop,recvpkt,(recvdrop*100/recvpkt));

					fprintf(stderr,"node> ");
					recvdrop=0;
					count=0;
					recvpkt=0;
					recvf=0;}else{
						printtime(timestamp);
						fprintf(stderr, "%s packet%d [\\n] received\n", timestamp, pkt.seq);
						printtime(timestamp);
						fprintf(stderr,"%s packet%d [\\n] discarded\n",timestamp,pkt.seq);
						memset(sen,0,sizeof(sen));
						strncpy(sen,"ACK",4);
						int now = seq-1;
						memcpy(sen+4,&now,sizeof(int));
						sendto(skt, sen, 10, 0, peeraddr,len);
						printtime(timestamp);
						fprintf(stderr,"%s ACK%d sent, expecting packet %d\n",timestamp, seq-1, seq);
					}
					
				}}else{
					printtime(timestamp);
					if(pkt.data!='\n'){
						fprintf(stderr,"%s packet%d %c discarded\n",timestamp,pkt.seq,pkt.data);
					}else{
						fprintf(stderr,"%s packet%d [\\n] discarded\n",timestamp,pkt.seq);
					}
					recvdrop++;
					recvpkt++;
				}
			}
		}//end if recv
	}
}



