#ifndef _FUNC_H_
#define _FUNC_H_

struct packet{
	char flag[4];
	int seq;
	unsigned short port;
	char data;
};

void printtime(char *timestamp);
int decide(char *mode, double dropv, int *count);

struct Node {
	unsigned short from;
	unsigned short to;
	double rate;
	unsigned short hop;
	struct Node *next;
};

struct info{
	unsigned short from;
	unsigned short to;
	double rate;
};

struct List {
	struct Node *head;
};

static inline void iniList (struct List *list){list->head=0;}

static inline int isEmptyList(struct List *list){return (list->head == 0);}

struct Node *copyNode(struct List *list, void *pointer);

int popFront(struct List *list);

void removeAllNodes(struct List *list);

void addlist(struct List *list, double x);

struct Node *findonto (struct List *list, unsigned short x);

int checkport(unsigned short x);

struct Node *loadnode(struct List *list, char *port, char *r, unsigned short m);

int countlist(struct List *list);

void addnode(struct List *list, struct Node *node, unsigned short myport);

void printlist(struct List *list);

int returnnumber (struct List *list, unsigned short x);

#endif
