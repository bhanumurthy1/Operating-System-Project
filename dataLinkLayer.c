#include<stdio.h>
#include<string.h>
#include"encDec.h"
//Frame data
void frameData(char * character_data) {
	int data_length = strlen(character_data);
	int length = 0;
	int i = 0;
	for (int i = 1; i < data_length ; i ++){
		length++;
		// Frame 32 characters 
		if (length == 32) {
			printf("222232");	
			length = 0;
			for (int j = (i-32); j < i; j++) {
				printf("%c",character_data[j]);
			}
		}
	}
	// Frame remaining characters 
	if(length != 0) {
		if(length < 9){
			printf("22220%d",length);
		}
		else {
			printf("2222%d",length);
		}
		for (int j = (data_length - length - 1); j < data_length; j++) {
			if(j >= 0) {
				printf("%c",character_data[j]);
			}
		}
	}
}
// Deframe data 
void deframeData(char * binary_data) {
	for(int i = 0; i < strlen(binary_data); ) {
		if(((i+3) < strlen(binary_data)) && binary_data[i] == '2' && binary_data[i+1] == '2' && binary_data[i+2] == '2' && 	binary_data[i+3] == '2'){
			i+=4;
		}
		else if((i>=4) && binary_data[i] >= '0' && binary_data[i] <= '9' && binary_data[i-1] == '2' && binary_data[i-2] <= '2' && 				binary_data[i-3] == '2' && binary_data[i-4] ){
			
			i+=2;
			
		}
		else{
			printf("%c",binary_data[i]);
			i++;
		}
	}
}
void dataLinkLayer(char * type, char * data) {
	// If value of type is "Frame", frame data
	if (strcasecmp(type,"Frame") == 0) {
		frameData(data);
	}
	// Else deframe data
	else {
		deframeData(data);
	}
}
int main(int argc, char * argv[]) {
	dataLinkLayer(argv[1],argv[2]);
	return 0;
}

