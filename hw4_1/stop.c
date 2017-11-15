#include <stdio.h>
#include <signal.h>

int sig_fn(int signo) {
	printf("Ctrl-C is pressed. Try Again\n");
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_fn);
	while (1) {
		continue;
	}

}
