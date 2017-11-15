#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <signal.h>

char filename[256];
int target_num = 300;
FILE* file = NULL;
pid_t pid_process1 = 0;
bool debug_mode = false;

void ProcessMain(pid_t next_pid, bool wakeup);

void ArgvAssign(int argc, char** argv){
	if(3 < argc && strcmp(argv[3], "debug") == 0){
		debug_mode = true;
	}	
	if (2 < argc) {		strcpy(filename, argv[2]);
	} else {			strcpy(filename, "sample.txt");
	}

	if (1 < argc) {		target_num = atoi(argv[1]);
	} else {			target_num = 300;
	}
}

int GetValueOnFile(){
	int value[1];
	fseek(file, 0, SEEK_SET);
	fread(value, sizeof(int), 1, file);
	return value[0];
}
void SetValueOnFile(int value){
	fseek(file, 0, SEEK_SET);
	fwrite(&value, sizeof(int), 1, file);
	//sleep(1);
}
void IsthisNeeded4Wakeup(int signo){
	//
}

int main(int argc, char** argv) {
	
	ArgvAssign(argc, argv);						// set count goal & file here
	printf("target_num==%d, filename==%s\n",target_num, filename);
	file = fopen(filename, "w+");
	SetValueOnFile(0);
	fseek(file, 0, SEEK_SET);

	signal(SIGUSR1, IsthisNeeded4Wakeup)						;

	pid_t pid = 0;
	pid_process1 = getpid();
	pid = fork();
	//printf("1\n");
	if(0 < pid){								// parent			
		ProcessMain(pid, false);				// process 1
	} else if (pid == 0){						// child1
		pid = fork();
		if(0 < pid){							// parent(child1)
			ProcessMain(pid, false);			// process 2
		} else if (pid == 0){					//child2
			ProcessMain(pid_process1, true);	// process 3
		} else {								// error
			printf("fork_error2\n");
			exit(1);
		}
	} else {									// error
		printf("fork_error1\n");
		exit(1);
	}
}

void ProcessMain(pid_t next_pid, bool wakeup){
	int count = 0;
	int process_num	= (int) (getpid() - pid_process1) % 3;
	int next_num	= (int) (next_pid - pid_process1) % 3;
	int status = 0;
	printf("process: %d => processMain(next = %d, wake = %s) start\n", process_num, (int)next_pid, wakeup? "true":"false");
	
	if(! wakeup){
		printf("process%d initially pause\n",process_num);
		pause();
	}
	while(true){
		if(debug_mode){
			printf("process%d wake up\n", process_num);
		}
		int value = GetValueOnFile();
		if(debug_mode){ 			printf("val = %d\n", value); }
		if(target_num <= value){
			break;
		}
		value ++;
		count ++;
		SetValueOnFile(value);
		fflush(file);
		/*	// I wanna wait until next process sleep. but this code not working
		do{
			if(debug_mode){ printf("checking next process%d is paused\n", next_num);}
			waitpid(next_pid, &status, WNOHANG);
		} while(! WIFSTOPPED(status));
		*/
		kill(next_pid, SIGUSR1);
		sleep(1);
	}
	printf("fin! process%d increase value %d times\n", process_num, count);
	kill(next_pid, SIGUSR1);
	exit(0);
}


