/* GBNclient.c */
/* Coauthor-ed by Chris Sterling and Chris Fichman */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>   /* memset() */
#include <sys/time.h> /* select() */
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "sendto_.h"
#include "packet.h"

char* read_file_into_memory(char* filename);

int main(int argc, char *argv[]) {
    
	/* check command line args. */
	if(argc<7)
	{
		printf("usage : %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
		exit(1);
	}

	/* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
	init_net_lib(atof(argv[3]), atoi(argv[4]));
	fd_set rdfs;
	struct timeval tv;
	struct Packet ACK;
	
	printf("error rate : %f\n",atof(argv[3]));

	/* socket creation */
	/*int sockets[WINDOW_SIZE];
	int i;
	for(i = 0; i < WINDOW_SIZE; i++) {
		if((sockets[i] = socket(AF_INET, SOCK_DGRAM, 0))<0)
		{
			printf("%s: cannot create socket \n",argv[0]);
			exit(1);
		}
		FD_SET(sockets[i], &rdfs);
	}*/
	int sd;
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	/*Wait up to TIMEOUT ms TODO - check if usec is microsec and convert */
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;
	
	/* get server IP address (input must be IP address, not DNS name) */
	struct sockaddr_in remoteServAddr;
	bzero(&remoteServAddr,sizeof(remoteServAddr));               //zero the struct
	remoteServAddr.sin_family = AF_INET;                 //address family
	remoteServAddr.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remoteServAddr.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	printf("%s: sending data to '%s:%s' \n", argv[0], argv[1], argv[2]);

	/* Call sendto_ in order to simulate dropped packets */
	int nbytes;
	struct Packet this_packet;
	int i;
	for(i=0;i<12;i++) {
		this_packet.seq_num = i;
		strcpy(this_packet.chunk, "Sending crap for our shit to see if shit is fucked.");
		strcat(this_packet.chunk, " ");
		//strcat(this_packet.chunk, (char)i);
		strcat(this_packet.chunk, "\n");
		nbytes = sendto_(sd, (void *)&this_packet, sizeof(this_packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
	}
	
	
	
	/*alarm(TIMEOUT/1000);
	
	if(errno == SIGALRM) {
		// alarm went off - resubmit packet
		nbytes = sendto_(sd, (void *)&this_packet, sizeof(this_packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
		alarm(TIMEOUT/1000);
	}*/
	
	//while(select(1, &rdfs, 0, 0, &tv) == 0) {
	//nbytes = recvfrom(sd, &ACK, sizeof(ACK), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
	//if(nbytes > 0) { 
		// ACK received.
		//printf("ACK Recieved.\n");
		//strcpy(ACK.chunk, ""); // DO WE NEED THIS?
		// Maybe we should be checking to see if the ACK.chunk == "ACK"???
		
		// TODO - move the window 
	//}
/*
 * while(1) {
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
 * */	
	
	// We have a window which contains WINDOWSIZE number of frames
	// We have each frame containing a Packet struct
	// We have a timer for each frame
	// Read in the first chunks from the file
	// For small scale purposes, let's use window size of 4
	// read 1 - send 1 - start timer1
	// read 2 - send 2 - start timer2
	// read 3 - send 3 - start timer3
	// read 4 - send 4 - start timer4
	
	// listen for ACK...
	// If ACK is received for leftmost value (LAR = last ack received), slide window
	
	// if ACK == LAR+1
		// then slide window
		// stop timer1
		// read 5 - send 5 - restart timer1
		// LAR++
		// LFS++
	// else - ACK is NOT L MOST value
		// Something bad happened or duplicate ACK received
		// No worries - no shit gets done here.
	// wait for any timer to explode
	
	// if any timer explodes, resend associated frame
	read_file_into_memory("wordlst.txt");

}


/* Reads a file into active memory. */
/* Possible Alternative - as we read the file, we send the file...gross... */
char*
read_file_into_memory(char* filename) {
	char full_file[MAX_FILE_SIZE]; //todo - dynamic memory?
	char chunk[MAX_FILE_CHUNK_SIZE];
	FILE *file_pointer;

	bzero(full_file, sizeof(full_file));
	bzero(chunk, sizeof(chunk));

	file_pointer = fopen(filename, "r");
	if (file_pointer == NULL)
	{
	  fprintf(stderr, "Cannot open input file %s! Sucks to be you nerd...\n", filename);
	  exit(EXIT_FAILURE);
	}
	
	int count=0;
	int i, file_pos, num_chunks, remainder;
	fseek(file_pointer, 0, SEEK_END);
	num_chunks = ftell(file_pointer) / MAX_FILE_CHUNK_SIZE;
	printf("%d", num_chunks);
	
	remainder = ftell(file_pointer) % MAX_FILE_CHUNK_SIZE;
	
	// Will send everything EXCEPT FOR the file remainder
	for(i = 0; i < num_chunks; i++) {
		bzero(chunk, sizeof(chunk));
		// Set it back to the start.
		rewind(file_pointer);
		file_pos = i * MAX_FILE_CHUNK_SIZE;
		fseek(file_pointer, file_pos, SEEK_CUR);
		fread(chunk, MAX_FILE_CHUNK_SIZE, 1, file_pointer);
		printf("CHUNK: %s\n\n", chunk);
	}
	
	/*
	while(!feof(file_pointer))
	{
		fread(chunk, MAX_FILE_CHUNK_SIZE, 1, file_pointer);
		//append chunk to file
		strncat(full_file, chunk, MAX_FILE_CHUNK_SIZE);
		bzero(chunk, MAX_FILE_CHUNK_SIZE);
		count++;
	}*/
	fclose(file_pointer);
	return full_file;
}

