//
// Created by Anuj Puri on 2024-03-28.
//

#include <sys/socket.h>
#include <netinet/in.h>

#ifndef FILE_DOWNLOAD_SERVER_SRVR_ATTRIBUTES_H
#define FILE_DOWNLOAD_SERVER_SRVR_ATTRIBUTES_H

#endif //FILE_DOWNLOAD_SERVER_SRVR_ATTRIBUTES_H

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 8080
#endif

struct sockaddr_in default_server_address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
};
