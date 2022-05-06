#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/times.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define DEFAULT "\033[30;1m"
#define RED "\033[31;1m"
#define GREEN "\033[32;1m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define GRAY "\033[38;1m"

/* File Descriptors for stdin and stdout */
#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2

/* Arbitrary rd_num size */
#define rd_num_SIZE 64

/* User read-write, group read, others read */
#define PERMS 0644

#define READ 0
#define WRITE 1

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("wrong arguments\n");
        exit(1);
    }
    int numberofchild = atoi(argv[1]);
    int paragontiko = atoi(argv[2]);
    if(paragontiko>16){
        perror("is too big\n");
        exit(1);
    }
    pid_t pid;
    int pipes[numberofchild + 1][2], wr_num, rd_num, k = 1, counter;
    for (int i = 0; i < numberofchild + 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe down 1\n");
            return 1;
        }
    }
    char str[5];
    char str1[5];
    strcpy(str,argv[1]);
    strcpy(str1,argv[2]);
    int j=0;
    while(str[j]!='\0'){
        if(isdigit(str[j])==0){
        perror("not numbers");
        exit(1);
        }
    j++;
    }
    j=0;
    while(str1[j]!='\0'){
        if(isdigit(str1[j])==0){
        perror("not numbers");
        exit(1);
        }
    j++;
    }
    if (paragontiko==1){
        printf("PARENT(%d): Factorial is: %d\n",getpid(),1);
        exit(0);
    }
    printf("PARENT=%d\n", getpid());
    for (int i = 0; i < numberofchild; i++)
    {
        pid = fork();
        usleep(50000);
        if (pid < 0)
        {
            perror("something went bad\n");
            return 2;
        }//child proces
		else if (pid == 0)
		{
			//printf("CHILD(%d)=%d\n",i,getpid());
			counter = i+1;     //counter = 1,2,3...,(n-1+1)=n after every fork +1
			k = counter;
	    if (counter == 1)
			{
				close(pipes[0][WRITE]); //1st child closes writing end of father
				if(read(pipes[0][READ], &rd_num, sizeof(rd_num))==-1)//1st child reads what father sent
                {
                    perror("pipe 1 down\n");
                    exit(1);
                }
				wr_num = rd_num * k;
				printf("CHILD(%d) read %d and give %d CHILD(%d)\n",i,rd_num,wr_num,i+1);
                close(pipes[1][READ]);
				if(write(pipes[1][WRITE], &wr_num, sizeof(wr_num))==-1){
                    perror("pipe 1 down\n");
                    exit(0);
                }
				k = k + numberofchild; // auksanei to k kata 1 gia na vrei epomeno fact
			}
		while (k <= paragontiko) //gia kathe epomeno paidi
			{
				if(counter==1)
				{
					close(pipes[numberofchild][WRITE]);
					if(read(pipes[numberofchild][READ], &rd_num, sizeof(rd_num))==-1){
                        perror("pipe 2 down\n ");
                        exit(1);
                    }
				}
				else
				{
					close(pipes[counter-1][WRITE]);// kathe paidi tha kleinei to writing end
					if(read(pipes[counter-1][READ], &rd_num, sizeof(rd_num))==-1)//kai tha diavazei apo to reading end
                    {
                        perror("pipe 3 donw\n");
                        exit(1);
                    }
				}
				wr_num = rd_num * k;
                if(counter<numberofchild)
				{
				printf("CHILD(%d) read %d and give %d CHILD(%d)\n",counter-1,rd_num,wr_num,counter);
				}
                else{
				printf("CHILD(%d) read %d and give %d CHILD(%d)\n",counter-1,rd_num,wr_num,0);
                }

                if (k == paragontiko)
				{
					printf("CHILD(%d): Factorial is: %d\n",counter,wr_num);//an to vrikes
					break;
				}
				else
				{
					k = k + numberofchild;// alliws sinexise na auksaneis 
					close(pipes[counter][READ]);
					if(write(pipes[counter][WRITE], &wr_num, sizeof(wr_num))==-1){
                        perror("pipe down 4\n");
                        exit(1);
                    }
				}
			}
			exit(0);
		}
	}

       //Main process
        for (int j = 0; j < numberofchild + 1; j++)
        {
            close(pipes[j][READ]);
            if (j != 0)
            {
                close(pipes[j][WRITE]);
            }
        }
        wr_num = 1;
        printf("Parent sent %d\n", wr_num);
        if (write(pipes[0][WRITE], &wr_num, sizeof(long int)) == -1)
        {
            perror("pipe down 5\n");
            return 5;
        }
        close(pipes[0][WRITE]);
        //printf("%d\n",n);
        for (int j = 0; j < numberofchild; j++)
        {
            wait(NULL);
        }
        return 0;
    }