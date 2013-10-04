//Server
#include<sys/types.h> 
#include<netinet/in.h> 
#include<arpa/inet.h> 
#define SIZE 4 
main()
{ 
	int std, lfd, len, i, j, status, sport; 
	char str[20], frame[20], temp[20], ack[20]; 
	struct sockaddr_in saddr, caddr; 
	printf("Enter the port address"); 
	scanf("%d", &sport); 
	std = socket(AF_INET, SOCK_STREAM, 0); 
	if(std<0) 
		perror("Error"); 
	bzero(&saddr, sizeof(saddr)); 
	saddr.sin_family = AF_INET; 
	saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	saddr.sin_port = htons(sport); 
	lfd = bind(std, (struct sockaddr *)&saddr, sizeof(saddr)); 
	if(lfd) 
		perror("Bind Error"); 
	listen(std, 5); 
	len = sizeof(&caddr); 
	lfd = accept(std, (struct sockaddr *)&caddr, &len); 
	printf("Enter the text:"); 
	scanf("%s", str);i = 0; 
	while( i<strlen(str) ) 
	{ 
		memset(frame, 0, 20); 
		strncpy(frame, str+i, SIZE); 
		printf("\nTransmitting frames:"); 
		len = strlen(frame); 
		for(j=0; j<len; j++) 
		{ 
			printf("%d", i+j); 
			sprintf(temp, "%d", i+j); 
			strcat(frame, temp); 
		} 
		write(lfd, frame, sizeof(frame)); 
		read(lfd, ack, 20); 
		sscanf(ack, "%d", &status); 
		if(status == -1) 
			printf("\nTransmission successful"); 
		else 
		{ 
			printf("Received error in: %d", status); 
			printf("\nRetransmitting frame"); 
			frame[0] = str[status]; 
			frame[1] = '\0';write(lfd, frame, sizeof(frame)); 
		} 
		i = i + SIZE; 
	} 
	write(lfd, "Exit", sizeof("Exit")); 
	printf("\nExitting!\n"); 
	sleep(2); 
	close(lfd); 
	close(std); 
} 



//client side: 
#include<stdio.h> 
#include<string.h> 
#include<sys/socket.h> 
#include<sys/types.h> 
#include<netinet/in.h> 
#include<arpa/inet.h> 
main() 
{ 
	int std, lfd, len, choice, cport; 
	char str[20], str1[20], err[20]; 
	struct sockaddr_in saddr, caddr; 
	printf("Enter the port address:"); 
	scanf("%d", &cport); 
	std = socket(AF_INET, SOCK_STREAM, 0); 
	if(std<0) 
		perror("Error"); 
	bzero(&saddr, sizeof(saddr)); 
	saddr.sin_family = AF_INET; 
	inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr); 
	saddr.sin_port = htons(cport); 
	connect(std, (struct sockaddr *)&saddr, sizeof(saddr)); 
	for(;;) 
	{ 
		read(std, str, 20); 
		if(strcmp(str, "Exit") == 0) 
		{ 
			printf("Exitting!\n"); 

			break; 
		} 
		printf("Received: %s\nError?(1 - YES or 0 - NO): ", str); 
		scanf("%d", &choice); 
		if(choice == 0) write(std, "-1", sizeof("-1")); 
		else
		{
			printf("Enter the sequence no of the frame where error has occured"); 
			scanf("%s", err); 
			write(std, err, sizeof(err)); 
			read(std, str, 20); 
			printf("Received the transmitted frame: %s\n", str); 
		} 
	} 
	close(std); 
}