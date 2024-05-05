#ifndef ERROR_CODES
#define EXIT_ERROR_CODE 99
#define EXIT_SUCCESS_CODE 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <limits.h>

//user header includes
#include "srvrutil.h"
#include "srvr_attributes.h"

/**
 * Configure the server, bind with the port and start listening for the
 * @param address_srvr
 * @param fd_sckt_srvr
 */
void start_server(const struct sockaddr_in address_srvr, int fd_sckt_srvr) {

    int fd_clnt_sckt = 0, no_of_clients = 0;
    socklen_t sckt_address_len = sizeof(address_srvr);

    // waiting for client
    printf("Waiting for client...\n");
    // wait in infinite loop totoke is accept the client input
    while (1)
    {

        if ((fd_clnt_sckt = accept(fd_sckt_srvr, (struct sockaddr *)&address_srvr, (socklen_t *)&sckt_address_len)) < 0)
        {
            perror("Error in accept");
            exit(EXIT_FAILURE);
        }

        // load balancing from server to mirror
        // if active clients less than =6 or is an odd no. after 12 connections
        // to be handled by server
        if ((((no_of_clients) % 9) / 3)  ==  0)
        {
            /// handle by server
            // sedn control message to client "CTS(Connected to server)"
            char msg[3] = "CTS";
            send_msg(fd_clnt_sckt, msg);

            printf("New connection from client: %s...\n", inet_ntoa(address_srvr.sin_addr));
            /// fork a child and call process client func
            pid_t pid = fork();
            if (pid == 0)
            {
                // child process
                close(fd_sckt_srvr);

                printf("client fd : %d", fd_clnt_sckt);
                // call process client function
                process_request(fd_clnt_sckt);
                exit(EXIT_SUCCESS);
            }
            else if (pid == -1)
            {
                // else failed to fork
                // error
                perror("Error processing client request");
                exit(EXIT_SUCCESS_CODE);
            }
            else
            {
                // parent process
                printf("closing the listening socket...");
                close(fd_clnt_sckt);
                while (waitpid(-1, NULL, WNOHANG) > 0); // clean up zombie processes
            }
        }
        else
        {
            // redirecting to mirror server
            printf("Redirecting to mirror\n");
            redirect_to_mirror(fd_clnt_sckt, no_of_clients);
        }

        // increase counter for no of connections
        no_of_clients = (no_of_clients + 1) % INT_MAX;
        printf("Client number : %d\n", no_of_clients);
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2 || argv == NULL) {
        perror(" bad argument list, port number is missing");
        exit(EXIT_FAILURE);
    }

    int fd_sckt_srvr = -1, _ret_val;
    int port_number = atoi(argv[1]);
    default_server_address.sin_port = htons(port_number);
    _ret_val = init_server(&fd_sckt_srvr, default_server_address, port_number);
    if(_ret_val < 0 || fd_sckt_srvr < 0) {
        perror("server init setup failed : ");
        exit(EXIT_FAILURE);
    }


    start_server(default_server_address, fd_sckt_srvr);

    printf("Hello World !!");
    return 0;
}
