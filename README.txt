-Leoni Lu
-jl5592

Commandline:
	$ make //would compile all the programs and generate three executable files: gbnnode, dvnode, cnnode
	$ ./gbnnode <selfport> <peerport> <windowsize> [ -d <value> | -p <value> ]
	$ ./dvnode <selfport> <neighport> <lossrate> ...[last]
	$ ./cnnode <selfport> receive <neighport> <lossrate> ... send <neighport> ... [last]

Data Structure:
	Linked list with node structure as follows:
		struct Node {
			unsigned short from;
			unsigned short to;
			double rate;
			unsigned short hop;
			struct Node *next;
		};//node structure for link information
		struct info{
			unsigned short from;
			unsigned short to;
			double rate;
		};
		struct List {
			struct Node *head;
		};
		struct packet{
			char flag[4];
			int seq;
			unsigned short port;
			char data;
		};//packet structure for gbnnode

GBNNODE:
	At the receiver side, I set all the variables, such as sequence number, to default, when the last packet, with data '\n' as the end of the packets, is received. As a result, if the last ACK is discarded by the sender, the sender may never finish sending the last packet, since the last ACK with right sequence number is never received. To avoid this, I add a terminating period for the receiver, which is set to [windowsize]*1000ms, right after the last ACK being sent out. In this period, the receiver would ACK back whatever packet it is received with the sequence number inside the packet, so that the receiver can received the last ACK even after the receiver received all packets and cleared the variables. 
	However, if the loss rate is too high, there is still chance that the sender discards a series of ACK during the whole terminating period. This may result in the sender falling into a loop of sending the last few packets. Add a terminating period would greatly avoid this infinite loop, but not always.

DVNODE:
	The routing table is printed everytime the node receives a routing table from neighbours, even though no data is updated. However, it will only resend the routing table if there is an update in the table. 

CNNODE:
	I did not finish this part because building on my existing code, I would have to greatly rewrite GBN algorithm to fit in the requirement. I used a single thread operation for gbnnode. For getting the loss rate every 1 sec for all neighbours, I would open multithread and run gbn algorithm for each neighbour and return new loss rate every 1 sec through a pipeline to cnnode. 
	Both Distance Vector and DV algorithm cases are implemented in the cnnode.
	cnnode would take in all commandline arguments, put the data into to linked list: neighbour, staticneighbour (neighbour would hold data for routing table; staticneighbour hold data for neighbours)
	If gbn-algorithm is implemented, the loss rate data would be updated into staticneighbour's rate field, which indicates the loss rate to neighbour. Then the cnnode can run normal dv algorithm upon two linked lists, neighbour & staticneibour.

Memory Issue:
	a cleanEXIT function is added to the program on signal SIGTERM and SIGINT. I hope this can help deallocating the memory on exit with ctrl C.	
