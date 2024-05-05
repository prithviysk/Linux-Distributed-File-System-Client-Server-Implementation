//
// Created by Anuj Puri on 2024-03-25.
//

#ifndef FILE_DOWNLOAD_SERVER_SRVRUTIL_H
#define FILE_DOWNLOAD_SERVER_SRVRUTIL_H

#define MAX_BUFFER_RR_SIZE 1024 // max buffer size for reading or writing the data from the Live socket.
#define MAX_QUEUE_SIZE 50 //defining the number of request queued, before rejection triggers.
#define _XOPEN_SOURCE 500
#define PORT 8080
#define MIRROR_PORT 7001
#define RESP_BUFFER_SIZE 256
#define BUFFER_FILE_NAME 64


//char request[MAX_BUFFER_RR_SIZE];
//char response[MAX_BUFFER_RR_SIZE*2];

#endif //FILE_DOWNLOAD_SERVER_SRVRUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>


#include "fileutil.h"
#include "mem_util.h"

//command literals
const char* CMD_LIST_DIR_SRTD_NAME = "dirlist -a";
const char* CMD_LIST_DIR_SRTD_MTIME = "dirlist -t";
const char* CMD_FILE_SRCH_NAME = "w24fn";
const char* CMD_FILE_SRCH_SIZE = "w24fz";
const char* CMD_FILE_SRCH_EXT = "w24ft";
const char* CMD_FILE_SRCH_DATE_BF = "w24fdb";
const char* CMD_FILE_SRCH_DATE_DA = "w24fda";
const char* CMD_CLIENT_QUIT = "quit";
const char* MSG_RES_REDIRECT = "BSY";
const char* MSG_RES_SERVER = "RESPONSE :\n";
const char* MSG_RES_SERVER_404 = "File not found.\n";
const char* MSG_RES_SERVER_INVALID_COMMAND = "Invalid command\n";
const char* MSG_RES_SERVER_ERR = " Server encountered an error.\n";
const char* LAST_MSG_END = "7@5!";

void handle_listdir_rqst(int fd, const char* command, char*** response);
void handle_fs_name(int fd, char** rqst);
void handle_fs_size(int fd, char** rqst, int n_args);
void handle_fs_date(int fd, char** rqst, int n_args, int dt_cmp);
void handle_fs_ext(int fd, char** rqst, int n_args);

/**
 *
 * @param fd_server
 * @param address_srvr
 * @param port_local
 * @return
 */
int init_server(int *fd_server, struct sockaddr_in address_srvr, int port);

//integer return
void process_request(int fd_clnt_sckt);
int send_msg_chars(int fd_sckt, char** msg, int msg_count);

int bind_address();
int listen_sckt();

/**
 * Sending an error message to teh client in case server
 * encounters an error or not a valid response.
 * @param fd_clnt_sckt
 * @param msg
 */
void send_msg(int fd_clnt_sckt, const char* msg) {
    if(msg != NULL) {
        if (send(fd_clnt_sckt, msg, strlen(msg), 0) < 0) {
            perror("Error : ");
        }
    }
}

/**
 * Sending a redirect signal to the client
 * @param fd_client
 */
void redirect_to_mirror(int fd_client, int n_clients) {
    int srvr_idx = (((n_clients) % 9) / 3);
    send_msg(fd_client, MSG_RES_REDIRECT);
    send_msg(fd_client, ulong_to_string(srvr_idx));
    close(fd_client);
}

void send_tar_to_client(int fd, char *tar_path) {
    int file = open(tar_path, O_RDONLY);
    // handle error
    if (file < 0)
    {
        perror("Error in opening the temp.tar.gz file");
    }

    char buffer[1024];
    ssize_t nBytes;
    while ((nBytes = read(file, buffer, sizeof(buffer))) > 0)
    {
        if (write(fd, buffer, nBytes) == -1)
            perror("Sending file failed");

        if (nBytes < sizeof(buffer))
            break;
    }
    close(file);
}

/**
 * Initializing the server by  opening a  server socket(file descriptor)
 *, binding to a port and start listening for the request from the client
 * @param fd_server
 * @param address_srvr
 * @return
 */
int init_server(int *fd_server, struct sockaddr_in address_srvr, int port) {

    int opt = 1; //option for server socket

    // Create socket file descriptor
    // use default protocol (i.e TCP)
    if ((*fd_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // error handle
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port 8080
    if (setsockopt(*fd_server, SOL_SOCKET, SO_REUSEADDR /*| SO_REUSEPORT*/, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    /// Attributes for binding socket with IP and PORT
    memset(&address_srvr, 0, sizeof(address_srvr));
    address_srvr.sin_family = AF_INET;
    address_srvr.sin_addr.s_addr = INADDR_ANY; // accepts any address
    address_srvr.sin_port = htons(port);

    // Bind socket to the PORT
    if (bind(*fd_server, (struct sockaddr *)&address_srvr, sizeof(address_srvr)) < 0)
    {	//error
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // listen_sckt to the socket
    // queue of size BACKLOG
    if (listen(*fd_server, MAX_QUEUE_SIZE) < 0)
    {
        perror("Error while listening..");
        exit(EXIT_FAILURE);
    }

    return 0;
}



void handle_listdir_rqst(int fd, const char* command, char*** response) {

    if(command == NULL) { return; }

    *response = NULL;
    char* dir_home = expand_if_tilda("~");
    printf("searching find in home %s\n", dir_home);

    struct dentry* list_dentry = NULL;
    int count_dir = -1;
    if(strcmp(CMD_LIST_DIR_SRTD_NAME, command) == 0) {
        count_dir = list_dir_sort(dir_home, &list_dentry, d_compare_name);
    } else if(strcmp(CMD_LIST_DIR_SRTD_MTIME, command) == 0){
        count_dir = list_dir_sort(dir_home, &list_dentry, d_compare_modified);
    }

    printf(" handle_listdir_rqst : found number of directories : %d\n", count_dir);
    if(list_dentry != NULL) {

        printf(" list is not NULL \n");
        *response = (char**)malloc(sizeof(char*) * count_dir);
        if (*response == NULL) {
            perror("malloc");
            return;
        }
        for(int i=0; i < count_dir; i++) {
            size_t length = strlen(list_dentry[i].f_name)+2;

            //create the response to be sent
            (*response)[i] = (char*) malloc(sizeof(char) * length);
//            //add the file name to response
            (*response)[i]  = strdup(list_dentry[i].f_name);
            (*response)[i][strlen(list_dentry[i].f_name)] = '\n';
            (*response)[i][length-1] = '\0';

        }

        for(int i = 0; i < count_dir; i++) {
            printf("copied directory is %s", (*response)[i]);
        }
        send_msg(fd, "TXT");
        send_msg_chars(fd, *response, count_dir);
    } else {
        send_msg(fd, MSG_RES_SERVER_404);
    }


    free(dir_home);
    free_dentry(list_dentry, count_dir);
}

void handle_fs_name(int fd_clnt_sckt, char** rqst) {
    // char* dir_home = expand_if_tilda("~");
    char* dir_home = "/Users/ysk/Desktop";

    char* file_name = calloc(sizeof(char), strlen(rqst[1]));
    strcpy(file_name, rqst[1]);

    //calling the function to check if the file exists
    //and get the required associated details.
    printf("Entering the file_search function\n");
    struct fdetails* fs_details_res = file_search(dir_home, file_name);
    printf("\n file name : %s\n", fs_details_res->f_name);
    printf(" file size : %s\n", fs_details_res->f_size);
    printf(" file permissions : %s\n", fs_details_res->f_mode);

    if(fs_details_res != NULL) {
        send_msg(fd_clnt_sckt, "TXT");

        ssize_t error = 0;
        error = write(fd_clnt_sckt, MSG_RES_SERVER, strlen(MSG_RES_SERVER)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, fs_details->f_name, strlen(fs_details->f_name)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, C_NEW_LINE, sizeof(char)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, fs_details->f_size, strlen(fs_details->f_size)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, C_NEW_LINE, sizeof(char)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, fs_details->f_mode, strlen(fs_details->f_mode)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, C_NEW_LINE, sizeof(char)) < 0 ? -1 : error;
        error = write(fd_clnt_sckt, fs_details->f_ctime, strlen(fs_details->f_ctime)) < 0 ? -1 : error;
        send(fd_clnt_sckt, LAST_MSG_END, strlen(LAST_MSG_END), 0);

        if (error != 0) {
            perror("Error while sending file details using name ");
        }
    } else {
        send_msg(fd_clnt_sckt, MSG_RES_SERVER_404);
    }

    free(dir_home);
    free(fs_details_res);
}

/*
 * reading request from the client
 */
int get_request(int fd_client, char* rqst, size_t sz_rqst) {

    ssize_t n_rb = read(fd_client, rqst, sz_rqst);
    printf("PRITHVI RQST: %s\n", rqst);
    if(n_rb <=0) {
        return -1;
    }

    return 0;
}

void handle_fs_size(int fd, char** rqst, int n_args) {
    printf("n args : %d,   %d   %d \n", n_args, is_number(rqst[1]), is_number(rqst[1]));
    if(n_args < 3 || !is_number(rqst[1]) || !is_number(rqst[2])) {
        perror("Invalid arguments to search for file using size");
        return;
    }

    // char* dir_home = expand_if_tilda("~");
    char* dir_home = "/Users/ysk/Desktop/ASP/";
    //TODO: REMOVE THIS DIR, IT IS FOR TESTING PURPOSE
    char* dir = "/Users/ysk/Desktop/ASP/";
    char* storage = "/Users/ysk/Desktop/ASP/tempstore_size/";

    printf("Entering the file_search function based on size\n");
    char* path_to_tar = file_search_size(dir, storage, atoll(rqst[1]), atoll(rqst[2]));

    if(path_to_tar != NULL) {
        write(fd, "TAR", 3);

        send_tar_to_client(fd, path_to_tar);
    } else {
        if(write(fd, MSG_RES_SERVER_404, strlen(MSG_RES_SERVER_404)) < 0) {
            perror("Error : ");
        }
    }

    free(dir_home);
    printf("comes here.... %s", path_to_tar);
}

void handle_fs_ext(int fd, char** rqst, int n_args) {
    if(n_args > 4) {
        perror("Invalid arguments to search for file using size");
        return;
    }

    char *ext[3];
    memset(ext, 0, sizeof(ext));
    for(int i = 0; i < n_args-1; i++) {
        ext[i] = strdup(rqst[i+1]);
    }

    printf("Entering the file_search with extension function\n");

    char* path_to_tar;
    char* dir_home = expand_if_tilda("~");
//    for(int i=1; i<n_args; i++){
        path_to_tar = f_extension_search("/Users/ysk/Desktop/ASP", "/Users/ysk/Desktop/ASP/tempstore_ext/", ext, n_args);
//    }
    printf("Tar path: %s\n", path_to_tar);

    write(fd, "TAR", 3);

    send_tar_to_client(fd, path_to_tar);
    for(int i = 0; i < 3 && ext[i] != NULL; i++) {
        free(ext[i]);
    }

}

void handle_fs_date(int fd, char** rqst, int n_args, int dt_cmp) {
    printf(" searchign with date %s\n", rqst[1]);
    char* dir_home = expand_if_tilda("~");

    char* dir = "/Users/ysk/Desktop/ASP";
    char* storage = "/Users/ysk/Desktop/ASP/tempstore_date/";

    printf("Entering into file_search_dt\n");
    char* path_to_tar = file_search_dt(dir, storage, rqst[1], dt_cmp);

    if(path_to_tar != NULL) {
        write(fd, "TAR", 3);

        send_tar_to_client(fd, path_to_tar);
        printf("comes here.... %s", path_to_tar);
    } else {
        if(write(fd, MSG_RES_SERVER_404, strlen(MSG_RES_SERVER_404)) < 0) {
            perror("Error : ");
        }
    }

    free(dir_home);
}


/**
 * Writing the response for the client.
 * @param fd_sckt
 * @param msg
 * @return
 */
int send_msg_chars(int fd_sckt, char** msg, int msg_count) {
    for(int i=0; i < msg_count; i++) {

        if((send(fd_sckt, msg[i], strlen(msg[i]), 0)) < 0) {
            perror("write failed ");
            return -1;
        }
    }
    printf("done with sending the data...");
    send(fd_sckt, LAST_MSG_END, strlen(LAST_MSG_END), 0);
    return 0;
}

/**
 * Processing the client request. This untility function is expected
 * to be called
 * @param fd_clnt_sckt file descriptor for client connection socket
 * @return 0 if the request is processed successfully, -1 otherwise.
 */
void process_request(int fd_clnt_sckt) {

    while(1) {
        char *rqst = malloc(sizeof(char) * MAX_BUFFER_RR_SIZE);
        memset(rqst, 0, sizeof(*rqst));
        if (get_request(fd_clnt_sckt, rqst, MAX_BUFFER_RR_SIZE) < 0) {
            perror("error reading request");
        }

        int num_tokens;
        char **cmd_vector = tokenize(rqst, C_SPACE, &num_tokens);
        if (cmd_vector == NULL || (strcmp(cmd_vector[0], CMD_CLIENT_QUIT) != 0 && num_tokens < 2)) {
            perror("error tokenizing request");
            send_msg(fd_clnt_sckt, MSG_RES_SERVER_ERR);
            continue;
        }

        printf("Request : %s\n", rqst);
        char **response = NULL;
        if (strcmp(rqst, CMD_LIST_DIR_SRTD_NAME) == 0) {
            handle_listdir_rqst(fd_clnt_sckt, CMD_LIST_DIR_SRTD_NAME, &response);
        } else if (strcmp(rqst, CMD_LIST_DIR_SRTD_MTIME) == 0) {
            handle_listdir_rqst(fd_clnt_sckt, CMD_LIST_DIR_SRTD_MTIME, &response);
        } else if (cmd_vector[0] != NULL && strcmp(cmd_vector[0], CMD_FILE_SRCH_NAME) == 0) {
            handle_fs_name(fd_clnt_sckt, cmd_vector);
        } else if (strcmp(cmd_vector[0], CMD_FILE_SRCH_SIZE) == 0) {
            handle_fs_size(fd_clnt_sckt, cmd_vector, num_tokens);
        } else if (strcmp(cmd_vector[0], CMD_FILE_SRCH_DATE_BF) == 0) {
            handle_fs_date(fd_clnt_sckt, cmd_vector, num_tokens, -1);
        } else if (strcmp(cmd_vector[0], CMD_FILE_SRCH_DATE_DA) == 0) {
            handle_fs_date(fd_clnt_sckt, cmd_vector, num_tokens, 1);
        } else if (strcmp(cmd_vector[0], CMD_FILE_SRCH_EXT) == 0) {
            handle_fs_ext(fd_clnt_sckt, cmd_vector, num_tokens);
        } else if (strcmp(cmd_vector[0], CMD_CLIENT_QUIT) == 0) {
            //Quit must release all resources exclusively
            free(rqst);
            free_array((void **) response);
            free_array((void **) cmd_vector);
            close(fd_clnt_sckt);
            exit(EXIT_SUCCESS);
        } else {
            send_msg(fd_clnt_sckt, MSG_RES_SERVER_INVALID_COMMAND);
        }

        //free up the memory
        printf("releasing memory...\n");
        free(rqst);
        free_array((void **) response);
        free_array((void **) cmd_vector);
        printf("done with processing the request ...\n");
    }

}


