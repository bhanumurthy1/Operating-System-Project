#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"encDec.h"
void applicationLayer(char * type, char * filename, char * data){
	// If value of type id "Read", read data from file
	if (strcasecmp(type,"Read") == 0) {
		FILE * file_pointer;
    		char data[127];
    		int num;
    		//Open file in read mode
    		file_pointer = fopen(filename, "r");
		if (file_pointer == NULL) {
			perror("Error occured while opening the file");
			exit(1);
		}
		// Read data
		while (fgets(data, 127, file_pointer) != NULL) {
			for(int i = 0; i < strlen(data); i++){
				num = data[i];
				if(num >= 0 && num <= 127){
        				printf("%c", data[i]);
        			}
        		}
    		}
		fclose(file_pointer);
	}
	// Write data to file
	else{
		// Open file in write mode
		FILE * file_pointer = fopen(filename, "w");
		if (file_pointer == NULL) {
			perror("Error occured while opening the file");
			exit(1);
		}
		// Write data
		fputs(data,file_pointer);
		fclose(file_pointer);
	}
}
int main(int argc, char * argv[]) {
	applicationLayer(argv[1], argv[2], argv[3]);
	return 0;
}

