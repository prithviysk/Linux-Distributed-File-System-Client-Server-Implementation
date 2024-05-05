//
// Created by Anuj Puri on 2024-04-08.
//

#ifndef FILE_DOWNLOAD_SERVER_FDETAILS_H
#define FILE_DOWNLOAD_SERVER_FDETAILS_H

#endif //FILE_DOWNLOAD_SERVER_FDETAILS_H

#include <time.h>


struct fdetails {
    char* f_name; //name of the file.
    char* f_mode; // file permissions
    char* f_size; //size of file in bytes
    char* f_ctime; //file creation time in nano seconds
    char* f_mtime; //file modified time in nano seconds
}f_details_entry;