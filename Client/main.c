#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_ARGS 10

char last_msg[4]="7@5!";
char command[];
char *args[MAX_ARGS];
void (*existing_handler)();

void tokenize(char *input){
    int num_args = 0;
    char *token = strtok(input, " ");
    strcpy(command, token);
    printf("Command: %s\n", command);
    char *arg = strtok(NULL, " ");
    while (arg != NULL && num_args < MAX_ARGS) {
        printf("Argument: %s\n", arg);
        args[num_args] = arg;
        arg = strtok(NULL, " ");
        num_args++;
    }
}

int validate_date(const char *dateStr) {
    struct tm timeStruct;

    // Parse the date string
    if (strptime(dateStr, "%d/%m/%Y", &timeStruct) == NULL) {
        return -1;
    } else {
        time_t currentTime = time(NULL);
        struct tm *localTime = localtime(&currentTime);
        int today_year = localTime->tm_year + 1900;
        int today_month = localTime->tm_mon + 1;
        int today_day = localTime->tm_mday;
        int year = timeStruct.tm_year + 1900;
        int month = timeStruct.tm_mon + 1;
        int day = timeStruct.tm_mday;
        if (month == 2) {
            if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
                if (day > 29) {
                    return -1;
                }
            } else {
                if (day > 28) {
                    return -1;
                }
            }
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            if (day > 30) {
                return -1;
            }
        } else if (day > 31) {
            return -1;
        }
        if (year > today_year ||
            ((year == today_year) && (month > today_month)) ||
            ((year == today_year) && (month == today_month) && (day > today_day))) {
            return -1;
        }
    }
    return 0;
}

int is_arg_number(const char *number_str) {
    int number = atoi(number_str);
    if (number <= 0) {
        return -1;
    }
    return 0;
}

int verify_input(){
    if (strcmp(command, "dirlist") == 0){
        if(strcmp(args[0], "-a") == 0) {
            return 0;
        } else if(strcmp(args[0], "-t") == 0) {
            return 0;
        } else {
            return -1;
        }
    } else if (strcmp(command, "w24fn") == 0) {
        return 0;
    } else if(strcmp(command, "w24fz") == 0) {
        int count = 0;
        for (int i = 0; args[i] != NULL && count < 2; i++) {
            if (is_arg_number(args[i]) == -1) {
                return -1;
            }
            count++;
        }
        if (count > 2) {
            printf("Too many arguments, expected less than 2\n");
            return -1;
        }
        return 0;
    } else if (strcmp(command, "w24ft") == 0) {
        int count = 0;
        for (int i = 0; args[i] != NULL && count < 2; i++) {
            count++;
        }
        if (count > 3) {
            return -1;
        }
        return 0;
    } else if (strcmp(command, "w24fdb") == 0) {
        printf(" comes before...\n");
        return(validate_date(args[0]));
    } else if (strcmp(command, "w24fda") == 0) {
        printf(" comes after...\n");
        return(validate_date(args[0]));
    } else if (strcmp(command, "quit") == 0) {
        signal(SIGINT, existing_handler);
        return 9;
    } else {
        printf("Command not part of server functionality\n");
        return -1;
    }
}

void receive_file(int file_fd, int socket)
{
    //check error
    if (file_fd < 0)
    {
        perror("Error creating file\n");
        return;
    }

    char buffer[1024];
    int bytesReceived;

    // read from the socket fd
    while ((bytesReceived = read(socket, buffer, 1024)) > 0)
    {
        // printf("%d received\n", bytesReceived);
        // write to the new tar file
        write(file_fd, buffer, bytesReceived);

        // if theno. of bytes received are less the buff size exit
        // this means there are no more character in socket
        if (bytesReceived < 1024)
            break;
    }

    printf("Done\n");
}

int main() {
    char input[256];
    char from_server[256];
    int spl_operation =0;
    char spl_char;
    char buffer[1024];
    char buffer_copy[1024];
    char response_t[4];
    int server_socket;
    int command_verification;
    int bytesReceived;
    struct sockaddr_in servAddr;
    int srvr_list[] = {8080, 8090,9000, 9010, 9020};
    int counter = 0;
//    existing_handler = signal(SIGINT,SIG_IGN);
    start:
    if ((server_socket= socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Failed to create socket\n");
        return 1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(srvr_list[counter]); // add port number
    servAddr.sin_addr.s_addr = INADDR_ANY;

    printf("trying to connect on port %d\n", srvr_list[counter]);
    if (connect(server_socket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

//    read(server_socket, buffer, 3);

    int n_bytes = recv(server_socket, response_t, sizeof(response_t) - 1, 0);
    response_t[n_bytes] = '\0';
    printf(" received msg : %s\n", response_t);
    if (strcmp(response_t, "BSY") == 0){
        printf("received from server %s", response_t);
        //current server is busy try the next one
        n_bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        buffer[n_bytes] = '\0';
        printf("received from server buffer %s", buffer);
        counter = atoi(buffer);
        close(server_socket);
        goto start;
    }
    while (1) {
            printf("client$ ");
            memset(buffer, 0, sizeof(buffer));

            fgets(buffer, 1024, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            strcpy(buffer_copy, buffer);
            if(strlen(buffer) == 0){
              printf("There has been an issue with the server. Please reconnect ....\n");
              break;
            }
            tokenize(buffer);
            command_verification = verify_input();
            printf(" verification : %d\n", command_verification);
            if(command_verification == 0){
                printf("Buffer: %s\n", buffer_copy);
                send(server_socket,buffer_copy, strlen(buffer_copy),0);

                memset(buffer, 0, sizeof(buffer));
                memset(response_t, 0, sizeof(response_t));
                int r_bytes = recv(server_socket, response_t, sizeof(response_t)-1, 0);
                response_t[r_bytes] = '\0';
                printf("Received Response:\n%s\n", response_t);

                if(strcmp(response_t, "TAR") == 0){
                    int fileDescriptor = open("temp.tar.gz", O_WRONLY | O_CREAT | O_TRUNC, 0777);

                    printf("Receiving the file ....\n");

                    receive_file(fileDescriptor, server_socket);
                    memset(buffer, '\0', sizeof(buffer));
                    //TODO:
                    //Have to check if it works for bigger tar file. More than the mentioned size

                    printf("File received\n");

                    close(fileDescriptor);
                } else if(strcmp(response_t, "TXT") == 0) {
                    printf("parsing text response\n");
                    bytesReceived = recv(server_socket, buffer, sizeof(buffer)-1, 0);
                    while(bytesReceived > 0){
                        buffer[bytesReceived] = '\0';
                        char* msg_end_rcvd = strdup(buffer+ (bytesReceived - 4));
                        if(strcmp(msg_end_rcvd, last_msg) == 0) {
                            buffer[bytesReceived-4] = '\0';
                            if(bytesReceived > (strlen(last_msg)+1)) {
                                printf("%s\n", buffer);
                            }
                            free(msg_end_rcvd);
                            break;
                        }
                        printf("%s", buffer);
                        free(msg_end_rcvd);
                        memset(buffer, 0, sizeof(buffer));
                        bytesReceived = recv(server_socket, buffer, sizeof(buffer) - 1, 0);

                    }
                } else if(strcmp(buffer, "ERR") == 0){
                    //ERROR HANDLING
                }
            } else if(command_verification == 9) {
                send(server_socket,buffer_copy, strlen(buffer_copy),0);
                memset(buffer, 0, sizeof(buffer));
                close(server_socket);
                printf("Connection is closed.\n");
                exit(EXIT_SUCCESS);
            }
        }

    return 0;
}