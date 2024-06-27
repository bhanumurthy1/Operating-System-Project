#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include"encDec.h"
#include<time.h>
// Generates error ar random position
void errorGenerator(int max, char * data, int noOfErrors){
	srand(time(NULL));
	for( int i = 0; i < noOfErrors; i++){
	    	int frameNumber = (rand() % (max + 1));
	    	int position;
	    	if(frameNumber < max) {
	    	position = (rand() % (37));
	    	}
	    	else {
	    	position = 4;
	    	}
	    	fprintf(stderr, "Error generated at frame %d at position - %d \n", frameNumber, position);
	    	// Flip the data at the genreated position
	    	data[frameNumber * 32 + position] = (data[frameNumber * 32 + position] == '1') ? '0' : '1';
	    	printf("%d %d\n", frameNumber, position);
	}    	
}
//Calculate CRC32 Checksum value
unsigned int crc32CheckSum(char * data){
	unsigned int crc = -1;
	const unsigned int crc_polynomial = 0x04C11DB7;
	for(int i = 0; i < strlen(data); i++) {
		crc = crc ^ data[i];
		for (int j = 0; j < 8; j++)
		{
		    crc = (crc << 1);
		    crc = (crc & 1);
		    if (crc > 0) {
		        crc = crc_polynomial;
		    }
		    else{
		    	crc = 0;
		    }
		}
	}
	return crc;
}
//Calculate Hamming Code
void calculateHammingCode(char * data) {
	int c ; 
	for(int i = 0; i < strlen(data); i++){
		if ((i+3) < strlen(data)){
		 c ^= (data[i+1] - '0') ^ (data[i+2] - '0') ^ (data[i+3] - '0');
		}
		else if ((i>=4) && data[i] >='0' && data[i] <= '9' && data[i-1] == '2' && data[i-2] =='2' && data[i-3]=='2' && data[i-4] =='2'){
				i += 2;
		}
		else{
			i++;
		}
	}
	
	
}
// Add odd parity bit to the binary value
void addParity(int input[7]){
	int parity_bit = 0, count = 0;
	// Count number of ones
	for (int i = 0; i < 7; i++) {
		if(input[i] == 1) {
			count++;
		}
	}
	// Set parity bit if number of ones is even 
	if (count % 2 == 0) {
		parity_bit = 1;
	}
	printf("%d",parity_bit);
	for(int i = 6; i >= 0; i--){
		printf("%d",input[i]);
	}
}

void integerToBinary(int input) {
	int k = 0;
	int binary[7] = {0};
	if(input < 9){
		addParity(binary);
	}
	// Convert integer to binary
	while(input != 0) {
		binary[k++] = input%2;
		input /= 2;
	}
	
	addParity(binary);
}
//Detect and Correct Errors
void detectAndCorrectErrors(char * data) {
	calculateHammingCode(data);
	char * token = strtok(data, " ");
	int first_num = atoi(token);
	token = strtok(NULL, " ");
	int second_num = atoi(token);
	printf("Error detected and corrected by HAMMING CODE at frame - %d at position - %d\n", first_num, second_num);
}
void characterToBinary(char input) {
	int ch = input;
	int binary[7] = {0}, i = 0;
	// Convert character to binary
	if (input != EOF && input != '\0') {
		while(ch != 0) {
			binary[i++] = ch%2;
			ch /= 2;
		}
		addParity(binary);
	}
}

void binaryToCharacter(int binary[7]){
	int number = 0, power_of_2 = 1;
	char ch;
	// Convert binary bits to character
	for(int i = 0; i < 7; i++){
		number += binary[i] * power_of_2;
		power_of_2 *= 2;
	}
	
	ch =  number;
	printf("%c", ch);
}
void binaryToInteger(int binary[7]){
	int number = 0, power_of_2 = 1;
	char ch;
	// Convert binary bits to integer
	for(int i = 0; i < 7; i++){
		number += binary[i] * power_of_2;
		power_of_2 *= 2;
	}
	printf("%d",number);
}

// Remove parity bit from binary value
void removeParity(char * input, char * type){
        int parity_bit = input[0], binary[7], count = 0, k = 0;
        for (int i = 7; i >= 1; i--) {
                if(input[i] == '1') {
                        count++;
                        binary[k++] = 1;
                }
                else {
                	binary[k++] = 0;
                }
        }
        // Check parity bit value
        if ((count % 2 == 0 && parity_bit == '1') || (count % 2 == 1 && parity_bit == '0') ) {
		if(strcasecmp(type,"Data") == 0){
			binaryToCharacter(binary);
		}
		else{
			binaryToInteger(binary);
		}
	}	
}
void physicalLayer(char * type, char * data, char * mode) {
	//If value of type is "encode", encode data
	if (strcasecmp(type,"Encode") == 0) {
		int errorDetectionMode = atoi(mode);
		int noOfErrors = 1;
		int j = 0, noOfFrames = 0; 
		if(errorDetectionMode > 0) {
			//Generate errors at random positions
			for (int i = 0; i < strlen(data);){
				if ((i+3) < strlen(data) && data[i] == '2' && data[i+1] == '2' && data[i+2] == '2' && data[i+3] == '2'){
					i += 4;
					noOfFrames++;
				}
				i++;
			}
			errorGenerator(noOfFrames, data, noOfErrors);	
			//CRC32 Error detection
			if(errorDetectionMode == 1) {
				crc32CheckSum(data);
			}
			else {
				// Hamming error detection and correction
				detectAndCorrectErrors(data);
			}
		} 
		else {
			for (int i = 0; i < strlen(data);){
				if ((i+3) < strlen(data) && data[i] == '2' && data[i+1] == '2' && data[i+2] == '2' && data[i+3] == '2'){
					integerToBinary(22);
					integerToBinary(22);
					i += 4;
					noOfFrames++;
				}
				else if ((i>=4) && data[i] >='0' && data[i] <= '9' && data[i-1] == '2' && data[i-2] =='2' && data[i-3]=='2' && data[i-4] =='2'){
						integerToBinary(((data[i]-'0') * 10) + (data[i+1]-'0'));
						i += 2;
				}
				else{
					characterToBinary(data[i]);
					i++;
				}
			}
		}
	}
	// Else decode data
	else {
		if(strcmp(mode, "1") == 0) {
		    	for(int i = 0; i < strlen(data); i++){
		    	}
		    	int data1 = crc32CheckSum(data);
		    	int data2 = crc32CheckSum(data);
		    	char * token = strtok(data, " ");
		    	data1 = atoi(token);
		    	token = strtok(NULL, " ");
		    	data2 = atoi(token);
		    	if(data1 != data2){
		    		printf("Error detected by CRC32 at frame - %d at position - %d\n", data1, data2);
		    	}
		}
		else if(strcmp(mode, "2") == 0){
			detectAndCorrectErrors(data);
		}
		else {
		int k = 0, count = 0;
		char binary[8];
		for (int i = 0; i < strlen(data); i += 8) {
	        	k = 0;
	        	count ++;
	        	for (int j = i; j < i+8; j++) {
	                	binary[k++] = data[j];
	        	}
	        	if ((count%35 >=1) && (count%35 <= 3)) {
	        		removeParity(binary, "Integer");
	        	}
	        	else {
	        		removeParity(binary, "Data");
	        	}
		}
		}
	}
}
int main(int argc, char * argv[]) {
	physicalLayer(argv[1],argv[2], argv[3]);
	return 0;
}

