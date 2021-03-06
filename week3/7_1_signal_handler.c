

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SIG_OK 0
#define SIG_NG -1
#define LOOP_ON 0
#define LOOP_OFF 1

int loopFlag = LOOP_ON;

// fuction to handle each signal
void sigHandleSIGALRM ();
void sigHandleSIGINT ();

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		printf("Argument should be 1 number\n");
		return -1;
	}

	signal(SIGALRM, sigHandleSIGALRM);
	signal(SIGINT, sigHandleSIGINT);

	printf("Cat is attacking! Ctrl - C quick!!!\n");

	// call alarm signal SIGALRM
	alarm(atoi(argv[1]));

	// infinity loop
	while (loopFlag == LOOP_ON) {
		sleep(1);
		printf(".\n");
	}
}

void sigHandleSIGALRM ()
{
	printf("CAT ATTACK!!!\n");
	usleep(500000);
	printf("You died.\n");
	exit(SIG_OK);
}

void sigHandleSIGINT ()
{
	printf("at died.\n");
	exit(SIG_OK);
}
