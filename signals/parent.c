/*
2h ergastiriaki askisi sta leitoyrgika systimata
Dimitrios Kalathas el18016
Dimitrios Kalemis el18152
*/

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

/* Arbitrary buffer size */
#define BUFFER_SIZE 64

/* User read-write, group read, others read */
#define PERMS 0644

int j = 0, w = 0, wstatus;
pid_t pidtable[100], cpid, pid2;
char buffer[100];
bool gate_state[100], father_order = false;

void check_neg(int ret, const char *msg) {
  if (ret < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

void describe_wait_status(pid_t pid, int status) {
  if (pid < 1) {
    perror("wait() call failed");
  }

  if (pid == 0) {
    printf("Nothing happened");
  }

  if (WIFSTOPPED(status)) {
    printf("Child with PID %d stopped\n", pid);
  } else if (WIFCONTINUED(status)) {
    printf("Child with PID %d continued\n", pid);
  } else if (WIFEXITED(status)) {
    printf("Child with PID %d exited with status code %d\n", pid,
           WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    printf("Child with PID %d terminated by signal %d with status code %d!\n",
           pid, WSTOPSIG(status), WEXITSTATUS(status));
  }
}

void forker(int nprocesses) {
    pid_t pid;
    
    if(nprocesses > 0) {
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(-1);
        }
        else if (pid == 0) {
            //Child stuff here
            char str[3];
            sprintf(str, "%d", w);
            if(buffer[w] == 't') {
                char *const argv[] = {"./child", str, "t", NULL};
                int status = execv("./child", argv);
                check_neg(status, "Failed to create child");
            }
            else {
                char *const argv[] = {"./child", str, "f", NULL};
                int status = execv("./child", argv);
                check_neg(status, "Failed to create child");
            }
        }
        else if (pid > 0) {
            //Parent Code
            pidtable[w] = pid;

            if (buffer[w] == 't') {
              gate_state[w] = true;
            }
            else {
              gate_state[w] = false;
            }

            usleep(50000);

            if (gate_state[w]) {
              printf(MAGENTA "[PARENT/PID=%d] Created child %d (PID=%d) and initial state 't'\n", getpid(), w, pidtable[w]);
            }
            else {
              printf(MAGENTA "[PARENT/PID=%d] Created child %d (PID=%d) and initial state 'f'\n", getpid(), w, pidtable[w]);
            }

            w++;
            forker(nprocesses - 1);
        }
    }
}

void handle_dad_signal(int sig) {
  switch(sig) {

    case SIGUSR1:
      for(int i = 0; i < w; i++) {
        kill(pidtable[i], SIGUSR1);
        usleep(5000);
      }
      break;

    case SIGUSR2:
      for(int i = 0; i < w; i++) {
        kill(pidtable[i], SIGUSR2);
        usleep(5000);
      }
      break;

    case SIGCHLD:
      if(father_order == false) {
        //printf("ENTER DAD HANDLER\n");
        //pid_t cpid, pid2;
        //int wstatus;

        //printf("BEFORE pid2=%d, cpid=%d, wstatus=%d\n", pid2, cpid, wstatus);
        pid2 = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);           //0
        //printf("AFTER pid2=%d, cpid=%d, wstatus=%d\n", pid2, cpid, wstatus);

        if (pid2 == -1) {
          perror("waitpid");
          exit(EXIT_FAILURE);
        }

        if (WIFSTOPPED(wstatus)) {                                 //CHILD STOPPED
          printf(MAGENTA "Child %d tried to stop but father caught it\n", pid2);
          kill(pid2, SIGCONT);
        }

        if (WIFEXITED(wstatus)) {                                 //CHILD IS TERMINATED
          int count = 0;
          bool check = true;
          while(check) {
            if(pidtable[count] != pid2) {
              count++;
            }
            else {
              check = false;
            }
          }
          printf(MAGENTA "[PARENT/PID=%d] Child %d with PID=%d exited\n", getpid(), count, pid2);
        
          //CREATE NEW CHILD
          cpid = fork();

          if ((cpid) < 0) {
            perror("fork");
            exit(-1);
          }

          else if (cpid == 0) {
            char str[3];
            sprintf(str, "%d", count);
            if(buffer[count] == 't') {
              char *const argv[] = {"./child", str, "t", NULL};
              int status = execv("./child", argv);
              check_neg(status, "Failed to create child");
            }
            else {
              char *const argv[] = {"./child", str, "f", NULL};
              int status = execv("./child", argv);
              check_neg(status, "Failed to create child");
            }
          }

          else {    
            if(buffer[count] == 't') {
              printf(MAGENTA "[PARENT/PID=%d] Created new child for gate %d (PID=%d) and initial state 't'\n", getpid(), count, cpid);
              fflush(stdout);         
            }
            else {
              printf(MAGENTA "[PARENT/PID=%d] Created new child for gate %d (PID=%d) and initial state 'f'\n", getpid(), count, cpid);
              fflush(stdout);  
            } 
          }
          pidtable[count] = cpid;
          cpid = wstatus = pid2 = 0;
        }
      }
      break;

    case SIGTERM:
      father_order = true;
      int w1 = w;
      for(int i = 0; i < w; i++) {
        usleep(50000);
        kill(pidtable[i], SIGTERM);
      }
      break;
  }
}

int main(int argc, char *argv[]) {

  //CHECK ARGUMENTS_________________________________________________________________________________________
    
  if (argc != 2) {
    perror("too many arguments or not enough arguments");
    exit(-1);
  }
  buffer[strlen(argv[1])];
  j = snprintf(buffer, strlen(argv[1])+1, "%s\n", argv[1]);
  //printf("%s\ncount = %d\n", buffer, j-1);                                                                        //prints argv string & number of children                                                      
  for (int i = 0; buffer[i] != '\0'; i++) {
    if ((buffer[i] == 't' || buffer[i] != 'f') && (buffer[i] != 't' || buffer[i] == 'f')) {
      perror("wrong arguments");
      exit(-1);
    }
  }

  //___________________________________________________________________________________________________________

  forker(j-1);

  struct sigaction sa;
  sa.sa_handler = &handle_dad_signal;
  sa.sa_flags = SA_RESTART; //what is this??

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGUS1"); // Should not happen
  }

  if (sigaction(SIGUSR2, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGUS2"); // Should not happen
  }

  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGTERM"); // Should not happen
  }

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGCHLD"); // Should not happen
  }
 
  while (1) {
    if(father_order == true) {
      for(int i = 0; i < w; i++) {
        int status;
        pid2 = waitpid(pidtable[i], &status, 0);
        printf(WHITE "[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), w-i);
        describe_wait_status(pid2, status);
      }
      printf(WHITE "All children exited, terminating as well\n");
    }
    pause();
  }
    
    return 0;
}