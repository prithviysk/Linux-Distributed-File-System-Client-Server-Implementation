//
// Created by Anuj Puri on 2024-04-08.
//

#include <string.h>
#include <stdlib.h>

#ifndef FILE_DOWNLOAD_SERVER_MEM_UTIL_H
#define FILE_DOWNLOAD_SERVER_MEM_UTIL_H

#endif //FILE_DOWNLOAD_SERVER_MEM_UTIL_H

void free_array(void** arr);

void free_array(void** arr) {
    if(arr == NULL) {
        return;
    }

    for(int i=0; arr[i] != NULL; i++) {
        if(arr[i] != NULL) {
            free(arr[i]);
        }
    }
    if(arr != NULL) {
        free(arr);
    }
}

void free_dentry(struct dentry *entry, int count) {

    for(int i=0; i < count; i++) {
        if(entry[i].f_name != NULL) {
            free(entry[i].f_name);
        }
    }

    free(entry);
}