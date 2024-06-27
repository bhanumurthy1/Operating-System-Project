#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/wait.h>
sem_t mutex[7];
int front = 0, rear = 0, sumOfDigits = 0;
char * buffer[10];
char * input_data;
char converted_data[10000];
// Replace a's with A in data
void * charA(void * arg) {
     int i = 0;
     while (i < strlen(input_data) && input_data[i] != '\0') {
     	char data[10000];
     	int j = 0;
     	while(input_data[i] != '\n') {
		    if (input_data[i] == 'a') {
		        data[j++] = 'A';
		    }
		    else{
		    	data[j++] = input_data[i];
		    }
		    i++;
        }
    	data[j++] = '\n';
    	data[j] = '\0';
    	i++;
    	// Enter critical section
    	sem_wait(&mutex[0]);
    	buffer[rear] = strdup(data);
    	rear = (rear + 1) % 10;
    	sem_post(&mutex[1]);
    	// Leave critical section     
     }
     sem_wait(&mutex[0]);
     buffer[rear] = "#";
     rear = (rear + 1) % 10;
     sem_post(&mutex[1]);
     return NULL;
}
// Replace e's with E in data
void * charE(void * arg) {
    while (1) {
    	// Enter critical section
        sem_wait(&mutex[1]);
        char * data = buffer[front];
        front = (front + 1) % 10;
	if (strcmp(data, "#") == 0) { 
		buffer[rear] = "#";
		rear = (rear + 1) % 10;
		sem_post(&mutex[2]);
		break;
	}
        for (int i = 0; i < strlen(data); i++) {
            if (data[i] == 'e') {
                data[i] = data[i] - 32;
            }
        }
        buffer[rear] = data;
        rear = (rear + 1) % 10;
        sem_post(&mutex[2]);
        // Leave critical section
    }
    return NULL;
}
// Replace i's with I in data
void * charI(void * arg) {
    while (1) {
    	// Enter critical section
        sem_wait(&mutex[2]);
        char * data = buffer[front];
        front = (front + 1) % 10;
	if (strcmp(data, "#") == 0) {
		buffer[rear] = "#";
		rear = (rear + 1) % 10;
		sem_post(&mutex[3]);
		break;
	}
        for (int i = 0; i < strlen(data); i++) {
            if (data[i] == 'i') {
                data[i] = data[i] - 32;
            }
        }
	buffer[rear] = data;
        rear = (rear + 1) % 10;
        sem_post(&mutex[3]);
        // Leave critical section
    }

    return NULL;
}
// Replace o's with O in data
void * charO(void * arg) {
    while (1) {
        // Enter critical section
        sem_wait(&mutex[3]);
        char * data = buffer[front];
        front = (front + 1) % 10;
	if (strcmp(data, "#") == 0) {
		buffer[rear] = "#";
		rear = (rear + 1) % 10;
		sem_post(&mutex[4]);
		break;
	}
        for (int i = 0; i < strlen(data); i++) {
            if (data[i] == 'o') {
                data[i] = data[i] - 32;
            }
        }
	buffer[rear] = data;
        rear = (rear + 1) % 10;
        sem_post(&mutex[4]);
        // Leave critical section
    }
    return NULL;
}
// Replace u's with U in data
void * charU(void * arg) {
    while (1) {
    // Enter critical section
        sem_wait(&mutex[4]);
        char * data = buffer[front];
        front = (front + 1) % 10;
	if (strcmp(data, "#") == 0) {
		buffer[rear] = "#";
		rear = (rear + 1) % 10;
		sem_post(&mutex[5]);
		break;
	}
        for (int i = 0; i < strlen(data); i++) {
            if (data[i] == 'u') {
                data[i] = data[i] - 32;
            }
        }
        buffer[rear] = data;
        rear = (rear + 1) % 10;
        sem_post(&mutex[5]);
        // Leave critical section
    }
    return NULL;
}
// Calculate sum of the digits in data
void * digit(void * arg) {
	while (1) {
		// Enter critical section
		sem_wait(&mutex[5]);
		char * data = buffer[front];
		front = (front + 1) % 10;
		if (strcmp(data, "#") == 0) {
			buffer[rear] = "#";
			rear = (rear + 1) % 10;
			sem_post(&mutex[6]);
			break;
		}
		for (int i = 0; i < strlen(data); i++) {
		    if (data[i] >= '0' && data[i] <= '9') {
		        sumOfDigits += data[i] -'0';
		    }
		}
		buffer[rear] = data;
		rear = (rear + 1) % 10;
		sem_post(&mutex[6]);
		// Leave critical section
	    }

    return NULL;
}
void * writer(void * arg) {
	while (1) {
	// Enter critical section
		sem_wait(&mutex[6]);
		char * data = buffer[front];
		front = (front + 1) % 10;
		if (strcmp(data, "#") == 0) {
			char message[100];
			sprintf(message, "Sum of digits is %d returned through the successful execution of the digit thread.", sumOfDigits);
			strcat(converted_data, message);
			sem_post(&mutex[6]);
			break;
		}
	      	strcat(converted_data, data);
		sem_post(&mutex[0]);
		// Leave critical section
	    }
    return NULL;
}
void error(const char *msg) {
	perror(msg);
	exit(1);
}
int main(int argc, char *argv[]){
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	char encoded_data[100000], decoded_data[100000], data[100000], data1[100000];
	int errorDetectionMode;
	int file_descriptor[2];
	pipe(file_descriptor);
	int status;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	fprintf(stdout, "Run client by providing host and port\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
	if (newsockfd < 0) {
		error("ERROR on accept");
	}
	
	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) {
		error("ERROR reading from socket");
	}
	bzero(data1,100000);
	n = read(newsockfd, data1, 99999);
	if (n < 0) {
		error("ERROR reading from socket");
	}
	if(strcmp(buffer,"Yes") == 0) {
		bzero(buffer,256);
		n = read(newsockfd,buffer,255);
		if (n < 0) {
			error("ERROR reading from socket");
		}
		if(strcmp(buffer,"Hamming Code") == 0) {
			errorDetectionMode = 2;
			printf("Error detection mode is: %s\n",buffer);
		}
		else {
			errorDetectionMode = 1;
			printf("Error detection mode is: %s\n",buffer);
		}
		
		int noOfErrors = 1;
		printf("No of errors : %d", noOfErrors);
	}
	bzero(encoded_data, 100000);
	// Read data from client
	n = read(newsockfd,encoded_data,100000);
	if (n < 0) {
		error("ERROR reading from socket");
	}
	if(errorDetectionMode == 1){
		// Decode data received from client
		if(execlp("./encodeDecode", "encodeDecode", "decode", "server", encoded_data, "1", NULL) < 0) {
			perror("Error orrured");
			exit(1);
		}
	}
	else if(errorDetectionMode == 2) {
		pid_t pid2 = fork();
		if (pid2 == 0) {
			// Decode data received from client
			if(execlp("./encodeDecode", "encodeDecode", "decode", "server", encoded_data, "2", NULL) < 0) {
				perror("Error orrured");
				exit(1);
			}
		}
		wait(&status);
	}
	else{
		pid_t  pid1 = fork();
		if (pid1 == 0) {
			close(file_descriptor[0]);
			if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
				perror("Error occured");
				exit(1);
			}
			close(file_descriptor[1]);
			// Decode data received from client
			if(execlp("./encodeDecode", "encodeDecode", "decode", encoded_data, NULL) < 0) {
				perror("Error orrured");
				exit(1);
			}
		}
		wait(&status);
	}
	
	input_data = data1;
	pthread_t tid[7];
	// Initialize semaphores
	sem_init(&mutex[0], 0, 1);
	for (int i = 1; i < 7; i++) {
	    	sem_init(&mutex[i], 0, 0);
	}
	// Thread creation
	pthread_create(&tid[0], NULL, charA, NULL);
	pthread_create(&tid[1], NULL, charE, NULL);
	pthread_create(&tid[2], NULL, charI, NULL);
	pthread_create(&tid[3], NULL, charO, NULL);
	pthread_create(&tid[4], NULL, charU, NULL);
	pthread_create(&tid[5], NULL, digit, NULL);
	pthread_create(&tid[6], NULL, writer, NULL);
	for (int i = 6; i >= 0; i--) {
	    	pthread_join(tid[i], NULL);
	}
	// Destory semaphores
	for (int i = 0; i < 7; i++) {
	    	sem_destroy(&mutex[i]);
	}		
	
	n = write(newsockfd, converted_data, strlen(converted_data));
	if (n < 0) {
		error("ERROR writing to socket");
	}
	pid_t  pid = fork();
	if (pid == 0) {
		close(file_descriptor[0]);
		// Redirect output to pipe
		if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
			perror("Error occured");
			exit(1);
		}
		close(file_descriptor[1]);
		// Encode converted data
		if(execlp("./encodeDecode", "encodeDecode", "encode", "server", "0", "0", converted_data, NULL) < 0) {
			perror("Error orrured");
			exit(1);
		}
	}
	else {
		wait(&status);
		read(file_descriptor[0], data, 100000);
		// Write data to client
		n = write(newsockfd, data, strlen(data));
		if (n < 0) {
			error("ERROR writing to socket");
		}
			
	}
	wait(&status);
	// Close socket
	close(newsockfd);
	close(sockfd);
	return 0;
}

