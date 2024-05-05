//
// Created by Anuj Puri on 2024-04-14.
//

#include <sys/time.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef FILE_DOWNLOAD_SERVER_DATEUTIL_H
#define FILE_DOWNLOAD_SERVER_DATEUTIL_H

#endif //FILE_DOWNLOAD_SERVER_DATEUTIL_H

int is_before(time_t date1, time_t date2);
int is_after(time_t date1, time_t date2);

time_t to_seconds(const char *date)
{
    struct tm storage={0,0,0,0,0,0,0,0,0};
    char *p=NULL;
    time_t retval=0;

    p=(char *)strptime(date,"%d/%b/%Y",&storage);
    if(p==NULL) {
        retval=0;
    } else {
        retval=mktime(&storage);
    }
    return retval;
}

int compare_dates_str(const char* date1, const char* date2) {
    time_t d1 = to_seconds(date1);
    time_t d2 = to_seconds(date2);

    return (d1 > d2) ? 1 : (d1 < d2 ? -1 : 0);
}


time_t get_file_creation_time(struct stat file_stat) {

     // Get creation time as time_t
    time_t creation_time = file_stat.st_birthtimespec.tv_sec;

    // Convert timestamp to formatted string
    struct tm *local_time = localtime(&creation_time);
    if (!local_time) {
        perror("localtime");
        return -1;
    }
    char buffer[20];
    strftime(buffer, 20, "%d/%m/%Y", local_time);

    return to_seconds(buffer);
}

char* tmtodt(struct stat file_stat) {

    // Get creation time as time_t
    time_t creation_time = file_stat.st_birthtimespec.tv_sec;

    // Convert timestamp to formatted string
    struct tm *local_time = localtime(&creation_time);
    if (!local_time) {
        perror("localtime");
        return NULL;
    }
    char buffer[20];
    strftime(buffer, 20, "%d/%m/%Y", local_time);

    return strdup(buffer);
}

int is_before(time_t f_date, time_t u_date) {
    return f_date <= u_date ? 0 : 1;
}

int is_after(time_t f_date, time_t u_date) {
    return f_date >= u_date ? 0 : 1;
}