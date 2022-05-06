#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define DEFAULT "\033[30;1m"
#define RED "\033[31;1m"
#define GREEN "\033[32;1m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define GRAY "\033[38;1m"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define PORT 18080

bool isNumeric(char str[])
{
    for (int i = 0; i < strlen(str); i++)
        if ((isdigit(str[i]) || str[i] == '\n' || str[i] == ' ') == false)
            return false; //when one non numeric value is found, return false
    return true;
}

bool flag = false;
char HOST[] = "lab4-server.dslab.os.grnetcloud.net";

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        perror("too much argumens\n");
        exit(1);
    }
    if (argc == 2)
    {
        if (strcmp(argv[1], "--debug") == 0 || strcmp(argv[1], "--DEBUG") == 0)
        {
            flag = true;
        }
        else
        {
            perror("wrong arguments\n");
            exit(1);
        }
    }

    //----------------creat socket-----------------//

    int domain = AF_INET;
    int type = SOCK_STREAM;
    int sock_fd = 0;

    if ((sock_fd = socket(domain, type, 0)) < 0)
    {
        perror("socket creation error\n");
        exit(1);
    }

    //-------------give addres in socket--------------//

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("something does bad with addres\n");
        exit(1);
    }

    //------------connection-------------------------//

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    //--conver ip addres to binary----//
    struct hostent *hostp;
    hostp = gethostbyname(HOST);
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length); ///selida 12 diafanies

    if (connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("lost connection\n");
        exit(1);
    }
    else
    {
        printf("connetion...\n");
    }

    //-------start to read from input or socket with select-----//

    char buffer[501];
    char *original_name;

    while (1)
    {
        fd_set inset;
        int maxfd;

        FD_ZERO(&inset);              // we must initialize before each call to select
        FD_SET(STDIN_FILENO, &inset); // select will check for input from stdin
        FD_SET(sock_fd, &inset);      // select will check for input from socket descriptor (Client)
        // -----select smaller than maxfd----//
        maxfd = MAX(STDIN_FILENO, sock_fd) + 1;

        int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
        if (ready_fds <= 0)
        {
            perror("select");
            continue; // just try again
        }

        //-----------read from input--------///

        if (FD_ISSET(STDIN_FILENO, &inset))
        {
            int n_read = read(STDIN_FILENO, buffer, 500);
            if (n_read == -1)
            {
                perror("read");
                exit(-1);
            }

            buffer[n_read] = '\0';
            // New-line is also read from the stream, discard it.
            if (n_read > 0 && buffer[n_read - 1] == '\n')
            {
                buffer[n_read - 1] = '\0';
                //memset(buffer, 0, sizeof(buffer));
            }

            if (n_read >= 4 && strncmp(buffer, "exit", 4) == 0)
            {

                int check = shutdown(sock_fd, 2); //2 giati den thelw oute read oute write
                if (check < 0)
                {
                    perror("shutdown");
                }
                printf("chao bella\n");
                close(sock_fd); //kleisimo kanaliou epikoinwnias
                exit(0);
            }

            else if (n_read >= 3 && strncmp(buffer, "get", 3) == 0)

            {
                int Wbytes = write(sock_fd, buffer, 3); //write() writes up to 3 bytes

                if (Wbytes < 0) //from the buffer to sd sock_fd
                {

                    perror("write");
                    return (-1);
                }
                if (flag)
                {
                    printf("[DEBUG] sent '%s'\n", buffer);
                }
            }

            else if (n_read >= 4 && strncmp(buffer, "help", 4) == 0) //if user types in Help
            {
                // user typed 'help'
                printf("Available commands:\n");
                printf("* 'help'                    : Print this help message\n");
                printf("* 'exit'                    : Exit\n");
                printf("* 'get'                     : Retrieve sensor data\n");
                printf("* 'N name surname reason'   : Ask permission to go out\n");
            }

            else if (n_read >= 1)
            {
                int Wbytes = write(sock_fd, buffer, n_read); //write() writes up to n_read bytes
                                                             //printf("%s\n",buffer );
                if (Wbytes < 0)
                {

                    perror("write");
                    return (-1);
                }

                if (flag)
                {
                    printf("[DEBUG] sent '%s'\n", buffer);
                }
            }
        }

        //-----------read from socket--------///

        if (FD_ISSET(sock_fd, &inset))
        {
            char bufferserver[500];
            int Rbytes = read(sock_fd, bufferserver, 500); // read() attempts to read up to 500 bytes from socket descr into the bufferserver

            if (strncmp(bufferserver, "try again", strlen("try again") - 1) == 0) //o server stelnei to try again
            {
                bufferserver[10]='\0';
                printf("%s", bufferserver);
                //fflush(stdin);
                memset(bufferserver, 0, sizeof(bufferserver)); //clear bufferserver
                continue;
            }
            else if (Rbytes >= 1 && isNumeric(bufferserver))
            {

                if (flag)
                {
                    printf("[DEBUG] read '%s'\n", bufferserver);
                }

                for (int i = 0; i < 20; ++i)
                {
                    printf("-");
                }
                printf("\n");
                char interval[40];
                char temperature[4];
                char light[40];
                char time[40];

                original_name = bufferserver; //pointer to 1st char element tou bufferserver
                strncpy(interval, original_name, 1);
                if (atoi(interval) == 0)
                {
                    printf("boot (0)\n");
                }
                if (atoi(interval) == 1)
                {
                    printf("setup(1)\n");
                }
                if (atoi(interval) == 2)
                {
                    printf("interval (2)\n");
                }
                if (atoi(interval) == 3)
                {
                    printf("button (3)\n");
                }
                if (atoi(interval) == 4)
                {
                    printf("motion (4)\n");
                }
                strncpy(light, original_name + 2, 3);
                printf("Light level is: %d\n", atoi(light));

                strncpy(temperature, original_name + 6, 4);
                printf("temperature is : %.2f\n", atof(temperature) / 100.0); 

                strncpy(time, original_name + 11, 10);
                time_t rawtime1 = atoi(time); //apo char se long_int
                struct tm *info;

                info = localtime(&rawtime1);
                printf("Current local time and date: %s", asctime(info));
                memset(bufferserver, 0, sizeof(bufferserver)); //clear bufferserver
            }
            else if (Rbytes >= 1 && strncmp(bufferserver, "ACK", strlen("ACK") - 1) == 0)
            {
                if (flag)
                {
                    printf("[DEBUG] read '%s'\n", bufferserver);
                }

                printf("Response:%s\n", bufferserver);
                memset(bufferserver, 0, sizeof(bufferserver)); //clear bufferserver
            }
            else if (Rbytes >= 1) //exoun apokleistei oi alles else if ara simainei oti o server exei steilei to verification code
            {
                if (flag)
                {
                    printf("[DEBUG] read '%s'\n", bufferserver);
                }

                printf("Send verification code: %s\n", bufferserver);
                memset(bufferserver, 0, sizeof(bufferserver)); //clear bufferserver
            }
        }
    }
    close(sock_fd);

   return 0;
}
