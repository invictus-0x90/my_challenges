#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <wait.h>

void flush();
bool play();
void admin(char *buff);

void admin(char *buff)
{
	char user[12];
	memset(user, sizeof(user), 0);
	memset(buff, sizeof(buff), 0);

	printf("[***] Welcome to the super secret admin panel [*]\n");
	printf("[***] This bonus area is all about custom shellcode ;)\n");
	printf("[***] Bonus bonus points for using info leak to bypass aslr, if your using the aslr version of this challenge\n");
	/* info leak */
	printf("[*] Username >>>");
	fgets(user, 11, stdin);
	flush();
	printf("\n:");
	printf(user);

	/* buffer overflow */
	printf("\n[***] Please Enter the secret passphrase [***]\n");
	printf(">>>");
	char *pass = buff;
	fgets(buff, 256, stdin);
	printf(": %s\n", buff);
}

void prompt(bool locked, char *buff)
{
	int choice = 0;
	flush();
	while(true)
	{
		if(locked)
			printf("[*] Menu: \n(1)Play a new game\n(2)Quit\n");
		else
		{
			printf("[*] Menu: \n(1)Play a new game\n(2)Quit\n");
			printf("(99)Admin panel\n");
		}

		printf(">>>");
		scanf("%d", &choice);
		flush(); //flush stdin
		switch(choice)
		{
			case 1:
				printf("[*] Starting new game![*]\n");
				printf("[*] Win 50 games to proceed[*]\n");
				locked = play();
				break;
			case 2:
				printf("[***] Exiting [***]\n");
				//exit(0);
				return;
			case 99:
				if(!locked)//double check they haven't cheated
				{
					char buff[128];
					admin(buff);
					return;
				}
				break;
			default:
				printf("[*] Not an option! [*]");
				break;
		}
	}
}

void flush()
{
	int ch;
	while((ch = fgetc(stdin) ) != EOF && ch != '\n');
}

bool play()
{
	char *RPS = "RPS";
	int wins = 0, rounds = 1;

	char their_guess;
	char my_guess;

	while(wins != 50)
	{
		printf("ROUND %d\n", rounds++);

		unsigned int random = rand() %3;
		my_guess = RPS[random];

		printf("Enter your choice: \n");
		scanf("%c", &their_guess);
		flush();
		printf("My choice:%c\nYour choice:%c\n", my_guess, their_guess);

		/* Go through game rules */
		if(my_guess == their_guess)
			printf("[*] Draw!! Try again [*]\n");
		else if(my_guess == 'R')
		{
			if(their_guess == 'S')
			{
				printf("[*] Rock beats scissors, you lose [*]\n");
				return true;
			}
			if(their_guess == 'P')
			{
				printf("[*] Paper beats rock! [*]\n");
				printf("[*] You win a point! [*]\n");
				wins++;
			}
		}
		else if(my_guess == 'P')
		{
			if(their_guess == 'R')
			{
				printf("[*] Paper beats rock, you lose [*]\n");
				return true;
			}
			if(their_guess == 'S')
			{
				printf("[*] Scissors beat paper [*]\n");
				printf("[*] You win a point! [*]\n");
				wins++;
			}
		}
		else if(my_guess == 'S')
		{
			if(their_guess == 'P')
			{
				printf("[*] Scissors beat Paper, you lose [*]\n");
				return true;
			}
			if(their_guess == 'R')
			{
				printf("[*] Rock beats scissors [*]\n");
				printf("[*] You win a point [*]\n");
				wins++;
			}
		}
		printf("[*] You have  %d points [*]\n", wins);
		fflush(stdout);
	}
	printf("[************************] YOU WIN!!! [*********************]\n");
	return false;
}

int main(void)
{
	/* get a handle to urandom */
	FILE *fp = fopen("/dev/urandom", "r");

	/* Bad stack setup */
	unsigned int seed = 0;
	char buff[50] = "";
	fread(&seed, 1, 4,fp); //get a seed

	printf("[*] Welcome to the ultimate Rock Paper Scissors Game [*]\n");
	printf("Please Enter a username >>");

	/* Overflow into the seed (woops) */
	fgets(buff, 54, stdin);
	printf("Welcome %s\n", buff);

	/* Double woops */
	srand(seed);

	/* Create a sandbox that forks here */
	/* Trace the process and stop any execve syscalls */
	int status = 0;
	int syscall = 0;
	pid_t pid = fork();

	if(pid == 0) //if we are the child process
	{
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL); //tell parent to trace child
		prompt(true, buff); //child executes rest of program
	}
	else //if we are the parent process
	{
		/* Monitor the child process, we are looking for syscalls or exit */
		while(true)
		{
			wait(&status);

			if(WIFEXITED(status) || WIFSIGNALED(status))
			{
				printf("[!] Child process exiting\n");
				break;
			}

			/* child gets trapped by kernel if it tries to make syscall */
			if(WSTOPSIG(status) == SIGTRAP)
			{
				syscall = ptrace(PTRACE_PEEKUSER, pid, 4*ORIG_EAX,NULL);
				struct user_reg_struct regs = ptrace(PTRACE_GETREGS, pid, &regs);
				
				/* get eax at the point of sigtrap */
				if(syscall == 11)
				{

					printf("Woooooahhhh no execve for you!\n");
					printf("[!] Try Harder ;) [!]\n");
					kill(pid, SIGKILL);
				}

			}
		}
	}
	return 0;

}