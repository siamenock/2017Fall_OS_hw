#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char filename[256];
int target_num = 300;
FILE* file = NULL;


void ArgvAssign(int argc, char** argv){
	if (2 <= argc) {
		strcpy(filename, argv[2]);
	}
	else {
		strcpy(filename, "default_file_name.txt");
	}

	if (1 <= argc) {
		target_num = atoi(argv[1]);
	}
	else {
		target_num = 300;
	}
}

int main(int argc, char** argv) {
	ArgvAssign(argc, argv);
	
	file = fopen(filename, "w+");
	pid_t pid;

	
	
	
	
}

