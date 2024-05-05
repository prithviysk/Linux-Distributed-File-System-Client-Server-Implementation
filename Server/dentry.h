//
// Created by Anuj Puri on 2024-04-06.
//

#ifndef FILE_DOWNLOAD_SERVER_FENTRY_H
#define FILE_DOWNLOAD_SERVER_FENTRY_H

#endif //FILE_DOWNLOAD_SERVER_FENTRY_H
#include <sys/stat.h>

struct dentry {
    char* f_name;
    struct stat stat;
}d_entry_details ;