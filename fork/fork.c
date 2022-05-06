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

#define DEFAULT "\033[30;1m"
#define RED "\033[31;1m"
#define GREEN "\033[32m"
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

/* Arbitrary buffer size */
#define BUFFER_SIZE 64

/* User read-write, group read, others read */
#define PERMS 0644


int main(int arc, char **argv) {
        time_t TIME;
        time(&TIME);
        int n = atoi(argv[2]);
        int fd = open(argv[1], O_RDWR | O_CREAT);
        if (fd == -1) {
                 perror("open");
                 exit(-1);
        }
        int i, k, status1;                                                                                      //status1 for child1
        pid_t x, y = fork(), v, pid_c1, pid_c2, check1 = 0, check2 = 0;
        char mypid1[40], mypid2[40], mypid3[40];
        srand(y);
        if (y == -1) {
                perror("fork");
        }
        else if (y == 0) {
                pid_c1 = getpid();
                printf(BLUE "[Child1]Started. PID=%d PPID=%d\n", pid_c1, getppid());
                for (int i = 0; i <= n; i += 2) {
                        printf(BLUE "[Child1]Heartbeat PID=%d, Time=%ld, x=%d\n", pid_c1, time(&TIME), i);
                        snprintf(mypid1, 40, "message from: %d\n", pid_c1);
                        if (write(fd, mypid1, strlen(mypid1)) != strlen(mypid1)) {
                                perror("write");
                                exit(-1);
                        }
                        sleep((rand() % 4) + 1);
                }
                printf(BLUE "[Child1] Terminating!\n");
                exit(0);
        }
        else {
                int status2;                                                                                    //status2 for child2
                x = fork();
                if (x == -1) {
                        perror("fork");
                }
                else if (x == 0) {
                        pid_c2 = getpid();
                        printf(RED "[Child2]Started. PID=%d PPID=%d\n", pid_c2, getppid());
                        for (int k = 1; k <= n; k += 2) {
                                printf(RED "[Child2]Heartbeat PID=%d, Time=%ld, x=%d\n", pid_c2, time(&TIME), k);
                                snprintf(mypid2, 40, "message from: %d\n", pid_c2);
                                if (write(fd, mypid2, strlen(mypid2)) != strlen(mypid2)) {
                                        perror("write");
                                        exit(-1);
                                }
                                sleep((rand() % 3) + 1);
                        }
                        printf(RED "[Child2] Terminating!\n");
                        exit(0);
                }
                else {
                        printf(DEFAULT "I am the Parent PID=%d\n", getpid());
                        for (int j = 0; j < n/2; j++) {
                                printf(DEFAULT "[Parent]Heartbeat PID=%d, Time=%ld\n", getpid(), time(&TIME));
                                snprintf(mypid3, 40, "message from: %d\n", getpid());
                                if (write(fd, mypid3, strlen(mypid3)) != strlen(mypid3)) {
                                        perror("write");
                                        exit(-1);
                                }
                                sleep((rand() % 2) + 1);
                        }
                        printf(DEFAULT "[Parent]waiting for my child\n");

                        wait(&status2);
                        printf(DEFAULT "child 2 terminated\n");

                        wait(&status1);
                        printf(DEFAULT "child 1 terminated\n");

                        printf(DEFAULT "[Parent]PID=%d Reading file:\n", getpid());

                        close(fd);
                        int n_read, n_write;
                        char buffer[BUFFER_SIZE];

  			//Open file for reading
                        int fd_in = open(argv[1], O_RDONLY);
                        if (fd_in == -1) {
                                perror("open");
                                exit(-1);
                        }

                        do {
                                //Read at most BUFFER_SIZE bytes, returns number of bytes read
                                n_read = read(fd_in, buffer, BUFFER_SIZE);
                                if (n_read == -1) {
                                        perror("read");
                                        exit(-1);
                                }

                                //Write at most n_read bytes, returns number of bytes written
                                n_write = write(FD_STDERR, buffer, n_read);
                                if (n_write < n_read) {
                                        perror("write");
                                        exit(-1);
                                }
                        } while (n_read > 0);

                        //Close input file
                        close(fd_in);
                }
        }
        return 0;
}