#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<fcntl.h>
#include<sys/wait.h>
void error(const char *msg) {
	perror(msg);
	exit(0);
}
int main(int argc, char *argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256], data[127];
	FILE * file_pointer;
	char * input_filename = "intext.txt";
	char encoded_data[100000], server_encoded_data[100000], decoded_data[100000], data1[100000];
	int file_descriptor[2], file_descriptor1[2];
	pipe(file_descriptor);
	pipe(file_descriptor1);
	int status;
	int noOfErrors = 0;
	int length = 0, errorChoice = 0, errorDetectionMode = 0;
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		error("ERROR connecting");
	}
	printf("Would you like to include errors \n1. Yes \n2. No\n");
	do {
		bzero(buffer,256);
		printf("Select 1 or 2: ");
		scanf("%d", &errorChoice);
		if(errorChoice == 1) {
			strcpy(buffer, "Yes");
		}
		else {
			strcpy(buffer, "No");
		}
	} while(errorChoice != 1 && errorChoice != 2);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0) {
		error("ERROR writing to socket");
	}
	pid_t pid = fork();
	if(pid == 0){
		close(file_descriptor1[0]);
		// Redirect output to pipe
		if (dup2(file_descriptor1[1], STDOUT_FILENO)<0) {
			perror("Error occured");
			exit(1);
		}
		close(file_descriptor1[1]);
		// Application Layer -- read data from file
		if(execlp("./applicationLayer", "applicationLayer", "Read", "intext.txt", NULL) < 0) {
			perror("Error orrured");
			exit(1);
		}
	}
	else{
		wait(&status);
		read(file_descriptor1[0], data1, 100000);
		n = write(sockfd, data1, strlen(data1));
		if (n < 0) {
			error("ERROR writing to socket");
		}
	}
	wait(&status);
	close(file_descriptor1[1]);
	close(file_descriptor1[0]);
	if(errorChoice == 1) {
		printf("Error detection modes \n1. CRC \n2. Hamming Code\n");
		do {
			bzero(buffer,256);
			printf("Enter error detection mode 1 or 2 : ");
			scanf("%d", &errorDetectionMode);
			if(errorDetectionMode == 1) {
				strcpy(buffer, "CRC");
			}
			else {
				strcpy(buffer, "Hamming Code");
			}
		}while(errorDetectionMode != 1 && errorDetectionMode != 2);
		
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) {
			error("ERROR writing to socket");
		}
		// Count number of frames
		length += strlen(data1);
		//Calculate number of frames;
		int noOfFrames = length/32;
		if (length < 32) {
			noOfFrames = 1;
		}
		noOfErrors = 1;
	}
	
	pid_t pid1 = fork();
	if (pid1 == 0) {
		close(file_descriptor[0]);
		// Redirect output to pipe
		if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
			perror("Error occured");
			exit(1);
		}
		close(file_descriptor[1]);
		char mode[10], errors[10];
		sprintf(mode, "%d", errorDetectionMode);
		sprintf(errors, "%d", noOfErrors);
		if(execlp("./encodeDecode", "encodeDecode", "encode", "client", mode, errors, NULL) < 0) {
			perror("Error orrured");
			exit(1);
		}
	}
	else {
		wait(&status);
		read(file_descriptor[0], encoded_data, 100000);
		// Write data to server
		n = write(sockfd, encoded_data, strlen(encoded_data));
		if (n < 0) {
			error("ERROR writing to socket");
		}	
	}
	wait(&status);
	n = read(sockfd, decoded_data, 10000);
	if (n < 0) {
		error("ERROR reading from socket");
	}
	if(errorDetectionMode != 1) {
		// Read data from server
		n = read(sockfd, server_encoded_data, 10000);
		if (n < 0) {
			error("ERROR reading from socket");
		}
		pid = fork();
		if (pid == 0) {
			close(file_descriptor[0]);
			// Redirect output to pipe
			if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
				perror("Error occured");
				exit(1);
			}
			close(file_descriptor[1]);
			// Decode data received from server 
			if(execlp("./encodeDecode", "encodeDecode", "decode", server_encoded_data, NULL) < 0) {
				perror("Error orrured");
				exit(1);
			}
		}
		else {
			wait(&status);
			// Create output file
			FILE * file_pointer = fopen("result.txt", "w");
			if (file_pointer == NULL) {
				perror("Error occured while opening the file");
				exit(1);
			}
			// Write output to file
			fputs(decoded_data,file_pointer);
			fclose(file_pointer);
			close(file_descriptor[1]);
			close(file_descriptor[0]);
				
		}
		printf("Output file has been generated\n");
		wait(&status);
	}
	close(sockfd);
	return 0;
}

