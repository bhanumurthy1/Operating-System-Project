#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
void encode(char * type, char * errorDetectionMode, char * noOfErrors, char * data){
	char framed_data[100000], encoded_data[100000];
	int file_descriptor[4];
	pipe(file_descriptor);
	pipe(file_descriptor + 2);
	pid_t pid;
	int status;
	if(strcmp(type, "client") == 0) {
		
		pid = fork();
		if (pid == 0) {
			close(file_descriptor[0]);
			// Redirect output to pipe
			if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
				perror("Error occured");
				exit(1);
			}
			for (int i = 0; i < 4; i++) {
				close(file_descriptor[i]);
			}
			// Application Layer -- read data from file
			if(execlp("./applicationLayer", "applicationLayer", "Read", "intext.txt", NULL) < 0) {
				perror("Error orrured");
				exit(1);
			}
		}
		else {
			wait(&status);
			pid = fork();
			if (pid == 0){
				close(file_descriptor[1]);
				close(file_descriptor[2]);
				// Read data from pipe
				read(file_descriptor[0], encoded_data, sizeof(encoded_data));
				// Redirect output to pipe
				if (dup2(file_descriptor[3], STDOUT_FILENO)<0) {
					perror("Error occured");
					exit(1);
				}
				for (int i = 0; i < 4; i++) {
					close(file_descriptor[i]);
				}
				// Data Link Layer -- frame data
				if (execlp("./dataLinkLayer", "dataLinkLayer", "Frame", encoded_data, NULL ) < 0){
					perror("Error orrured");
					exit(1);
				}
			}
			else {	
				wait(&status);
				pid = fork();
				if (pid == 0) {
					close(file_descriptor[3]);
					close(file_descriptor[4]);
					// Read data from pipe
				 	read(file_descriptor[2], framed_data, sizeof(framed_data));
				 	for (int i = 0; i < 4; i++) {
						close(file_descriptor[i]);
					}
					// Physical Layer -- encode data
					if (execlp("./physicalLayer", "physicalLayer", "Encode", framed_data, errorDetectionMode, NULL ) < 0){
						perror("Error orrured");
						exit(1);
					}
				}
			}
		}
	}
	else{
		pid = fork();
		if (pid == 0){
			close(file_descriptor[0]);
			if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
				perror("Error occured");
				exit(1);
			}
			for (int i = 0; i < 4; i++) {
				close(file_descriptor[i]);
			}
			// Data Link Layer -- frame data
			if (execlp("./dataLinkLayer", "dataLinkLayer", "Frame", data, NULL ) < 0){
				perror("Error orrured");
				exit(1);
			}
		}
		else {	
			wait(&status);
			pid = fork();
			if (pid == 0) {
			 	read(file_descriptor[0], framed_data, sizeof(framed_data));
			 	for (int i = 0; i < 4; i++) {
					close(file_descriptor[i]);
				}
				// Physical Layer -- encode data
				if (execlp("./physicalLayer", "physicalLayer", "Encode", framed_data, errorDetectionMode, NULL ) < 0){
					perror("Error orrured");
					exit(1);
				}
			}
			else{
				wait(&status);
			}
		}
	}
	// Close file descriptors of pipes
	for (int i = 0; i < 4; i++) {
		close(file_descriptor[i]);
	}
	wait(&status);
	wait(&status);
}
void decode(char * type, char * data, char * errorDetectionMode){
	char deframed_data[100000], decoded_data[100000];
	int file_descriptor[2], status;
	pipe(file_descriptor);
	pid_t pid;
	if(strcmp(type,"client") == 0) {
		pid = fork();
		if (pid == 0) {
			close(file_descriptor[0]);
			// Redirect output to pipe 
			if (dup2(file_descriptor[1], STDOUT_FILENO)<0) {
				perror("Error occured");
				exit(1);
			}
			close(file_descriptor[1]);
			// Physical Layer - decode data
			if(execlp("./physicalLayer", "./physicalLayer", "Decode", data, errorDetectionMode, NULL) < 0) {
				perror("Error orrured");
				exit(1);
			}
		}
		else{ 
			wait(&status);
			pid = fork();
			if (pid == 0){
				close(file_descriptor[1]);
				// Read data from pipe
				read(file_descriptor[0], decoded_data, sizeof(decoded_data));
				close(file_descriptor[0]);
				// Data Link Layer -- deframe data 
				if (execlp("./dataLinkLayer", "dataLinkLayer", "Deframe", decoded_data, NULL ) < 0){
					perror("Error orrured");
					exit(1);
				}
			}
			else{
				wait(&status);
			}
		  	
		}
	}
	else {
		// Physical Layer - decode data
		if(execlp("./physicalLayer", "./physicalLayer", "Decode", data, errorDetectionMode, NULL) < 0) {
			perror("Error orrured");
			exit(1);
		}
	}
	// Close file descriptors of pipes
	for (int i = 0; i < 2; i++) {
		close(file_descriptor[i]);
	}
	wait(&status);
}
int main(int argc, char * argv[]){
	if (strcmp(argv[1], "encode") == 0) {
		encode(argv[2], argv[3], argv[4], argv[5]);
	}
	else {
		decode(argv[2], argv[3], argv[4]);
	}
	return 0;
}
