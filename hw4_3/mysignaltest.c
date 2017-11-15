#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

static jmp_buf env_alrm, goto_back;

static void sig_alrm(int signo);
static void sig_int(int signo);
unsigned int sleep2(unsigned int seconds);

int main(void) {
	unsigned int unslept;

	if (signal(SIGINT, sig_int) == SIG_ERR){
		fprintf(stderr, "signal(SIGINT) error");
		exit(-1);
	}
	
	unslept = sleep2(5);

	printf("sleep2 returned: %u\n", unslept);
	return 0;
}

static void sig_alrm(int signo)
{
	longjmp(env_alrm, 1);
}

static void sig_int(int signo)
{
	int i,j;
	volatile int k;

	printf("\nsig_int starting\n");
	alarm(0);						//disable alarm
	/*
	 * 아래 for문이 5초 이상 실행되도록 적당히 바꿔주세요. 
	 */
	for (i = 0; i<3000000; i++)
		for (j = 0; j<4000; j++){
			k += i*j;
			k -= i*j;
		}
	printf("sig_int finished\n");
	longjmp(env_alrm, 10);
	
	
}

unsigned int sleep2(unsigned int seconds){
	int jumper_val = 0;
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		return seconds;

	if ((jumper_val = setjmp(env_alrm)) == 0){
		alarm(seconds);
		pause();
	} else {	// called by sig_int
		//do nothing
	}
	return alarm(0);
}
