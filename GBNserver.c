/* GBNserver.c */
/* This is a sample UDP server/receiver program */
/* This code will not work unless modified. */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <time.h>
#include "sendto_.h"
//#include "swp.h"
#include "packet.h"

#define TIMEOUT 50 // 50ms timeout
#define WINDOW_SIZE 3 //TODO - make this less arbitrary


int main(int argc, char *argv[]) {

	/* check command line args. */
	if(argc<6) {
		printf("usage : %s <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
		exit(1);
	}
	printf("error rate : %f\n",atof(argv[2]));

	/* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
	init_net_lib(atof(argv[2]), atoi(argv[3]));
	printf("error rate : %f\n",atof(argv[2]));

	int sd;
	/* socket creation */
	if((sd=socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		printf("%s: cannot open socket \n",argv[0]);
		exit(1);
	}

	/* bind server port to "well-known" port whose value is known by the client */
	struct sockaddr_in servAddr;
	bzero(&servAddr,sizeof(servAddr));                    //zero the struct
	servAddr.sin_family = AF_INET;                   //address family
	servAddr.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	servAddr.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	if(bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr))<0)
	{
		printf("%s: cannot to bind port number %d \n", argv[0], atoi(argv[1]));
		exit(1); 
	}

	/* Receive message from client */
	struct sockaddr_in cliAddr;
	unsigned int cliLen;
	int nbytes;
	struct Packet recvmsg;
	struct Packet ACK;
	strcpy(ACK.chunk, "ACK");
	//bzero(recvmsg, sizeof(recvmsg));
	cliLen = sizeof(cliAddr);
	//nbytes = recvfrom(sd, &recvmsg, sizeof (recvmsg), 0, (struct sockaddr *) &cliAddr, &cliLen);
	fd_set rdfs; 
	struct timeval tv;
	FD_SET(sd, &rdfs);
	
	/*Wait up to TIMEOUT ms TODO - check if usec is microsec and convert */
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;
	printf("The value of select() is %d\n", select(1, &rdfs, 0, 0, &tv));
	while(1) {
		if(select(1, &rdfs, 0, 0, &tv) > 0) {
			nbytes = recvfrom(sd, &recvmsg, sizeof(recvmsg), 0, (struct sockaddr *) &cliAddr, &cliLen);
			if(nbytes > 0) { 
				// Send ACK
				ACK.seq_num = recvmesg.seq_num;
				nbytes = sendto_(sd, (void*)ACK, sizeof(ACK),0, (struct sockaddr *) &cliAddr, sizeof(cliLen));
				break; 
			}
		}		
	}
	printf("%s recieved.\n", recvmsg.chunk);
	/* Respond using sendto_ in order to simulate dropped packets */
	//char response[] = "respond this";
	
}
