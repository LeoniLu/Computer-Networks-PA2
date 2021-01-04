#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "func.h"

void printtime(char *timestamp){
	struct timeval t;
        gettimeofday(&t,0);
        double time = (double)t.tv_sec + (double)t.tv_usec/1000000;
        sprintf(timestamp,"[%.6f]",time);
}

int decide(char *mode, double dropv,int *count){
	if(!strcmp(mode,"-d")){
		int d = (int)dropv;
		*count = *count+1;
		if(*count>=d){
			*count = 0;
			return 0;
		}else{
			return 1;
		}
	}else if(!strcmp(mode, "-p")){
		double r = ((double) rand() / (RAND_MAX));
		if(r<dropv){return 0;}else{return 1;}
	}
	return 0;
}

struct Node *copyNode(struct List *list, void *pointer){
	//take a pointer, memory copy the node into the linked list
	struct Node *node = malloc(sizeof(struct Node));
	if(!node){return NULL;}
	memcpy(node, pointer, sizeof(struct info));
	node->hop = 0;
	node->next = list->head;
	list->head = node;
	return node;
}

int popFront(struct List *list){
	if(isEmptyList(list)){return 0;}
	struct Node *node = list->head;
	list->head = node->next;
	free(node);
	return 1;
}

void removeAllNodes(struct List *list){
	while(!isEmptyList(list)){popFront(list);}
}

void addlist(struct List *list, double x){
	struct Node *node = list->head;
	while(node){
		node->rate = node->rate+x;
		node = node->next;
	}
}

struct Node *findonto (struct List *list, unsigned short x){
	struct Node *node = list->head;
	while(node){
		if(node->to==x){return node;}
		node = node->next;
	}
	return NULL;
}

int checkport(unsigned short x){
	if(x>=1024&&x<=65534){return 1;}
	else{return 0;}
}

struct Node *loadnode(struct List *list, char *port, char *r, unsigned short m){
	unsigned short nport = atoi(port);
	double ra = strtod(r, NULL);
	if(!checkport(nport)){return NULL;}
	struct Node *node = malloc(sizeof(struct Node));
	node->from = m;
	node->to = nport;
	node->rate = ra;
	node->hop = 0;
	node->next = list->head;
	list->head = node;
	return node;
}

int countlist(struct List *list){
	struct Node *node = list->head;
	int x = 0;
	while(node){
		node = node->next;
		x++;
	}
	return x;
}

void addnode(struct List *list, struct Node *node, unsigned short myport){
	struct Node *addn = malloc(sizeof(struct Node));
	addn->from = myport;
	addn->to = node->to;
	addn->rate = node->rate;
	addn->hop = node->from;
	addn->next = list->head;
	list->head = addn;
}


void printlist(struct List *list){
	struct Node *node = list->head;
	fprintf(stderr, "Node %d Routing Table\n",node->from);
	while(node){
		fprintf(stderr, "- %.4f -> Node %d",node->rate, node->to);
		if(node->hop >0){
			fprintf(stderr," ; Next hop -> Node %d",node->hop);
		}
		fprintf(stderr,"\n");
		node = node->next;
	}
}

int returnnumber (struct List *list, unsigned short x){
	struct Node *node = list->head;
	int count = 0;
	while(node){
		if(node->to == x){return x;}
		count++;
		node = node->next;
	}
	return 0;
}
