//
// Created by Anuj Puri on 2024-03-28.
//
#ifndef FILE_DOWNLOAD_SERVER_FILEUTIL_H
#define FILE_DOWNLOAD_SERVER_FILEUTIL_H
#define MAX_BUFFER_FILE_SIZE 256
#define MAX_BUFFER_NAME 128

//user defined directives
#define CODE_ERROR_APP  (-99)
#define CODE_FILE_FOUND 999
#define F_OPEN_LIMIT 100
#define MAX_BUFFER_SIZE 512
#endif //FILE_DOWNLOAD_SERVER_FILEUTIL_H

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>
#include <time.h>
#include <sys/syslimits.h>
#include "dateutil.h"
// USER INCLUDES
#include "dentry.h"
#include "fdetails.h"
#include "stringutils.h"


//directive required for nftw() function
#define _XOPEN_SOURCE 600


//declaring global variables

//constant global variables
const char* msg_file_not_found = "Search Unsuccessful.";

//string literals
const char* cmd_tar = "tar -cf ";
const char* name_tar = "temp.tar.gz";
const char c_extsn = '.'; // to find the file extension
char *d_storg_path = NULL;

//global variables
char *f_path_tracker = NULL;

char* user_input_file_name =  NULL;
char** usr_f_extension = NULL;
struct fdetails* fs_details;

//integer variables.
int found_file;
ssize_t min_f_sz = 0;
ssize_t max_f_sz = 1024;
time_t ufs_ctime = 0;

//integer returning functions.


int create_temp_tar_gz(const char *dirpath);
int d_compare_name(const void *entry1, const void *entry2);
int d_compare_modified(const void *x_time, const void *y_time);
int copy_file_fd(int fd_src, int fd_dest);
int open_file(const char* f_path, int oargs);
int dir_count(char* d_path);
int list_dir_sort(char *d_path, struct dentry** list, int (*sort_compare)(const void*, const void*));
int f_callback_bsize(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw);
int f_callback_name(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw);
int f_callback_dt(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw);
int f_callback_extsn(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw);

void copy_file(const char* src_file, const char* dest_file);

struct fdetails* file_search(const char* root_path, const char* file_name);
char* file_search_size(const char* root_path, char* storage_path, int sz_min, int sz_max);
char* file_search_dt(const char* root_path, const char* storage_path, const char* date, int dt_cmp);

//string returning functions.
char* expand_if_tilda(char *command);


//function pointer
void (*fn_cp_handle)(const char* , const char*);
int (*compare_date)(time_t date1, time_t date2);


/**
 * This function implements the requirement of searching a given file name inside the
 * provided path, recursively. It uses the standard library function nftw(), which
 * does the recursive walk.
 * @param root_path
 * @param file_name
 * @return
 */
struct fdetails* file_search(const char* root_path, const char* file_name) {

    //let's free up the memory to make sure there are no leaks.
    //don't want to take the risk of freeing up the same memory twice.
    //it's not recommended at all as it can lead to unidentified behavior.
    if(f_path_tracker != NULL) {
        free(f_path_tracker);
    }

    //allocating new memory, we definitely need it to save the path
    //I mean absolute path of the file.
    //the responsibility of free-ing up this memory lies on the calling method
    //as this reference is returned.
    f_path_tracker = malloc(sizeof(char) * PATH_MAX);

    //hate the null checks but need it...
    if(root_path == NULL || file_name == NULL) {
        perror("invalid arguments supplied, operation failed.");
        exit(CODE_ERROR_APP);
    }

    printf("ROOT PATH: %s\n", root_path);

    //tracking the user
    fs_details = malloc(sizeof(f_details_entry));
    user_input_file_name = file_name;
    printf("Entering the callback for file search function\n");
    int code = nftw(root_path, f_callback_name, F_OPEN_LIMIT, FTW_PHYS);
    if(code == -1) {
        printf("\n%s ", msg_file_not_found);
        exit(CODE_ERROR_APP);
    }

    if(found_file != 1) {
        printf("\n%s ", msg_file_not_found);
    }

    if(code == CODE_FILE_FOUND) {
        printf("%s", " Search successful. ");
        return fs_details;
    } else {
        free(f_path_tracker); //in case of returning NULL, let's free up the memory.
        return NULL;
    }
}


/**
 * This function implements the requirement of searching a given file name inside the
 * provided path, recursively. It uses the standard library function nftw(), which
 * does the recursive walk.
 * @param root_path
 * @param file_name
 * @return
 */
char* file_search_size(const char* root_path, char* storage_path, const int sz_min, const int sz_max) {

    //let's free up the memory to make sure there are no leaks.
    //don't want to take the risk of freeing up the same memory twice.
    //it's not recommended at all as it can lead to unidentified behavior.
    if(f_path_tracker != NULL) {
        free(f_path_tracker);
    }

    //allocating new memory, we definitely need it to save the path
    //I mean absolute path of the file.
    //the responsibility of free-ing up this memory lies on the calling method
    //as this reference is returned.
    f_path_tracker = malloc(sizeof(char) * PATH_MAX);

    printf("inside the file_search with size function\n");

    //handling the checks to
    if(root_path == NULL
            || storage_path == NULL
            || sz_min < 0
            || sz_max < sz_min
            || sz_max < 0) {
        perror("invalid arguments supplied, operation failed.");
        return NULL;
    }

    //tracking the user
    //setting variable to be used by the callback method
    d_storg_path = strdup(storage_path);
    min_f_sz = sz_min;
    max_f_sz = sz_max;
    printf("Entering the file_search with size callback function\n");
    int code = nftw(root_path, f_callback_bsize, F_OPEN_LIMIT, FTW_PHYS);
    if(code == -1) {
        printf("\n%s ", msg_file_not_found);
        return NULL;
    }


    if(f_path_tracker != NULL) {
        free(f_path_tracker); //in case of returning NULL, let's free up the memory.
    }

    int is_tar = create_temp_tar_gz(d_storg_path);

    char* tar_path = malloc(sizeof(char) * (strlen(d_storg_path)+ strlen(name_tar)+1));
    if(is_tar == 0) {
        strcpy(tar_path, d_storg_path);
        strcat(tar_path, name_tar);
    } else {
        free(tar_path);
        return NULL;
    }
    return tar_path;
}

/**
 * This function implements the requirement of searching a given file name inside the
 * provided path, recursively. It uses the standard library function nftw(), which
 * does the recursive walk.
 * @param root_path
 * @param file_name
 * @return
 */
char* file_search_dt(const char* root_path, const char* storage_path, const char* date, int dt_cmp) {
    printf(" searchign with date\n");
    //let's free up the memory to make sure there are no leaks.
    //don't want to take the risk of freeing up the same memory twice.
    //it's not recommended at all as it can lead to unidentified behavior.
    if(f_path_tracker != NULL) {
        free(f_path_tracker);
    }

    //allocating new memory, we definitely need it to save the path
    //I mean absolute path of the file.
    //the responsibility of free-ing up this memory lies on the calling method
    //as this reference is returned.
    f_path_tracker = malloc(sizeof(char) * PATH_MAX);

    //hate the null checks but need it...
    if(root_path == NULL || date == NULL) {
        perror("invalid arguments supplied, operation failed.");
        return NULL;
    }

    //tracking the user
    //setting variable to be used by the callback method
    fs_details = malloc(sizeof(f_details_entry));
    ufs_ctime = to_seconds(date);
    d_storg_path = strdup(storage_path);
    //set the comparator according to the requirement
    if(dt_cmp < 0) {
        compare_date = is_before;
    } else if(dt_cmp > 0) {
        compare_date = is_after;
    }

    printf("Entering into date callback function\n");
    int code = nftw(root_path, f_callback_dt, F_OPEN_LIMIT, FTW_PHYS);
    ufs_ctime = 0; //reset the value as it is a global variable.

    if(code == -1) {
        printf("\n%s ", msg_file_not_found);
        return NULL;
    }

    free(f_path_tracker); //in case of returning NULL, let's free up the memory.

    printf("generating the path to tar...");
    int is_tar = create_temp_tar_gz(d_storg_path);

    char* tar_path = malloc(sizeof(char) * (strlen(d_storg_path)+ strlen(name_tar)+1));
    if(is_tar == 0 && tar_path != NULL) {
        strcpy(tar_path, d_storg_path);
        strcat(tar_path, name_tar);
    } else {
        free(tar_path);
        return NULL;
    }
    printf("returing the path to tar...");
    return tar_path;
}

/**
 * function to handle the callback from "not file tree walk" nftw()
 * this shall be called everytime a directory or file is encountered.
 * The function will handle callbacks only for type "file" or F_FTW
 * @param path_current
 * @param f_stat
 * @param f_type
 * @param ptr_struct_ftw
 * @return
 */
int f_callback_name(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw) {
    if(f_type == FTW_F) {

        //use the base to find the current file name, add base to f_current address
        //FTW is a structure, where base is a member variable
        //base contains the location of the base file_name
        //this is just a position, when you add base to path_current
        //basically you are doing a pointer arithmetic operation
        //and then f_current contains the pointer to location
        //in a char* where the file_name starts.
        const char *f_current = path_current + ptr_struct_ftw->base;
        printf("inside the file_search callback\n");

        if (user_input_file_name != NULL && f_current != NULL && strcmp(f_current, user_input_file_name) == 0) {
            //once the file is found, returning non-zero value to stop the traversal.
            realpath(path_current, f_path_tracker);

            //enabling a boolean as the callback will return to nftw() only
            //though nftw will also return the same code but need to
            // return some other value from the caller function.
            if(fs_details != NULL) {
                fs_details->f_name = strdup(f_current);
                fs_details->f_size = strdup(ulong_to_string(f_stat->st_size));
                fs_details->f_mode = strdup(get_permissions(f_stat->st_mode));
                fs_details->f_ctime = tmtodt(*f_stat);
            }
            found_file = 1;
            printf("\n%s %s ", "Search successful. Absolute path is - ", f_path_tracker);
            return CODE_FILE_FOUND;
        }
    }

    //Returning '0' so that nftw() contrinues the traversal to find the file.
    return 0;
}

/**
 * function to handle the callback from "not file tree walk" nftw()
 * this shall be called everytime a directory or file is encountered.
 * The function will handle callbacks only for type "file" or F_FTW
 * @param path_current
 * @param f_stat
 * @param f_type
 * @param ptr_struct_ftw
 * @return
 */
int f_callback_extsn(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw) {
    if(f_type == FTW_F) {
        //use the base to find the current file name, add base to f_current address
        //FTW is a structure, where base is a member variable
        //base contains the location of the base file_name
        //this is just a position, when you add base to path_current
        //basically you are doing a pointer arithmetic operation
        //and then f_current contains the pointer to location
        //in a char* where the file_name starts.
        const char *f_current = path_current + ptr_struct_ftw->base;

        printf("inside the file_search with extensino callback function\n");

        int match_found = 0;
        if(usr_f_extension != NULL ) {
            for (int i = 0; i < 3; i++) {
                if (usr_f_extension[i] != NULL && (extcmp(f_current, usr_f_extension[i]) == 0)) {
                    printf("extension: %s\n", usr_f_extension[i]);
                    match_found = 1;
                    break;
                }
            }
        }

        if(match_found) {
            realpath(path_current, f_path_tracker);
            printf("\n%s %s ", "Search successful. Absolute path is - ", f_path_tracker);

            //if this function pointer is not NULL, that means the call is made to nftw() via
            //f_extension_search, which indeed requires to not just find the files with provided extension
            //but also process them to create a .tar file
            //this function will help to copy all the files found as per extension match to a
            //storage directory
            if(d_storg_path != NULL) {

                char* temp_path = malloc(sizeof(char) * (strlen(d_storg_path) + strlen(f_current) + 2));
                strcpy(temp_path, d_storg_path);

                strcat(temp_path, "/");
                strcat(temp_path, f_current);

                printf("\n storage path : %s\n", temp_path);
                copy_file(f_path_tracker, temp_path);

                free(temp_path);
                printf(" \nresuming storage path : %s\n", d_storg_path);
            }

            //not returning anything here. The search must be continued for all the files.
            //in case of extension file search.

        }
        // }
    }

    //Returning '0' so that nftw() contrinues the traversal to find the file.
    return 0;
}


/**
 * function to handle the callback from "not file tree walk" nftw()
 * this shall be called everytime a directory or file is encountered.
 * The function will handle callbacks only for type "file" or F_FTW
 * and only record the files which have specific size limits.
 * @param path_current path to the current file.
 * @param f_stat stat variable for the file
 * @param f_type type of the file
 * @param ptr_struct_ftw
 * @return
 */
int f_callback_bsize(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw) {
    if(f_type == FTW_F) {

        //use the base to find the current file name, add base to f_current address
        //FTW is a structure, where base is a member variable
        //base contains the location of the base file_name
        //this is just a position, when you add base to path_current
        //basically you are doing a pointer arithmetic operation
        //and then f_current contains the pointer to location
        //in a char* where the file_name starts.
        const char *f_current = path_current + ptr_struct_ftw->base;
        if (f_current != NULL && (f_stat->st_size >= min_f_sz && f_stat->st_size <= max_f_sz)) {
            printf("inside the file_search with size callback function\n");
            realpath(path_current, f_path_tracker);
            //start copying the file into storage directory
            char* dest_path = malloc(sizeof(char) * (strlen(f_current) + strlen(d_storg_path)+1));
            strcpy(dest_path, d_storg_path);
            strcat(dest_path, f_current);
//            printf("\ncurrent path %s", f_path_tracker);
//            printf("\nStorage directory is  %s, and file name is %s , original name %s", d_storg_path, dest_path, f_current);
            copy_file(f_path_tracker, dest_path);
            free(dest_path);
        }
    }

    //Returning '0' so that nftw() contrinues the traversal to find the file.
    return 0;
}

int f_callback_dt(const char *path_current, const struct stat *f_stat, int f_type, struct FTW *ptr_struct_ftw) {
    if(f_type == FTW_F) {

        //use the base to find the current file name, add base to f_current address
        //FTW is a structure, where base is a member variable
        //base contains the location of the base file_name
        //this is just a position, when you add base to path_current
        //basically you are doing a pointer arithmetic operation
        //and then f_current contains the pointer to location
        //in a char* where the file_name starts.
        const char *f_current = path_current + ptr_struct_ftw->base;

        printf("Inside the callback\n");

//        time_t f_stat->st_ctimespec.tv_sec;

        if(compare_date(f_stat->st_birthtimespec.tv_sec,ufs_ctime)) {
            realpath(path_current, f_path_tracker);
            printf("\n%s %s ", "Search successful. Absolute path is - ", f_path_tracker);

            //if this function pointer is not NULL, that means the call is made to nftw() via
            //f_extension_search, which indeed requires to not just find the files with provided extension
            //but also process them to create a .tar file
            //this function will help to copy all the files found as per extension match to a
            //storage directory
            if(d_storg_path != NULL) {

                char* dest_path = malloc(sizeof(char) * (strlen(d_storg_path) + strlen(f_current)+1));
                strcpy(dest_path, d_storg_path);
                strcat(dest_path, f_current);

                printf("\n storage file path : %s\n", dest_path);
                copy_file(f_path_tracker, dest_path);
                free(dest_path);
                //putting a logic to reuse the existing string memory.
                printf(" \nresuming storage path : %s\n", d_storg_path);
            } else {
                return -1;
            }

            //not returning anything here. The search must be continued for all the files.
            //in case of extension file search.

        }
    }

    //Returning '0' so that nftw() contrinues the traversal to find the file.
    return 0;
}

/*
 *  A utility function to copy file from one location to
 *  another location.
 */
void copy_file(const char* src_file, const char* dest_file) {

    printf("\n source file %s, and destination file %s", src_file, dest_file);

    int fd_src = open_file(src_file, O_RDONLY);
    int fd_dest = open_file(dest_file, O_RDWR);
    printf("Inside the copy file function\n");

    //in case of any error, report and exit.
    if(fd_src == -1) {
        printf("\n%s", "file copying failed. source file does not exist.");
        exit(CODE_ERROR_APP);
    } else if(fd_dest == -1 ) {
        fd_dest = open_file(dest_file, O_CREAT|O_RDWR);
        if(fd_dest == -1) {
            printf("\n%s", "file copying failed. cannot create destination file.");
            exit(CODE_ERROR_APP);
        }
    }

    //now, let's start the process of reading the data from the src file
    copy_file_fd(fd_src, fd_dest);
}

//print absolute path using nftw(), and callback() functions
char* f_extension_search(const char* root_path, const char* storage_path, char *f_extension[], int n_args) {

    //opening these fds just to make sure the paths exist
    //don't want to call nftw() with NULL values
    //though it may handle, but i don't know the internal implementation of nftw()
    //so not relying on it.
    int fd_root = open_file(root_path, O_RDONLY);
    printf(" fd_root %d, root_path %s\n", fd_root, root_path);
    if(fd_root == -1) {
        perror(" f_extension_search: root path not found.");
        exit(CODE_ERROR_APP);
    }

    if(NULL == storage_path) {
        perror("error occurred");
        exit(CODE_ERROR_APP);
    }

    printf("Inside the file_search with extension function\n");

    //allocating dynamic memory for managing the destination path for
    //copying or moving the file.
    char * f_dest_path = malloc(sizeof(char)*(strlen(storage_path)));
    realpath(storage_path, f_dest_path);

    printf("f_dest_path: %s\n", f_dest_path);

    struct stat status_block;

    if(stat(f_dest_path, &status_block) == 0 && S_ISDIR(status_block.st_mode)) {
        printf("%s ", f_dest_path);
        //DO NOTHING WE JUST NEED TO CREATE STORAGE PATH
    } else {
        if(mkdir(f_dest_path, 0777) == 0) {
            printf("Created directory\n");
            //DO NOTHING WE JUST NEED TO CREATE STORAGE PATH
        } else {
            perror(" f_extension_search: storage path not found.");
            exit(CODE_ERROR_APP);
        }
    }

    //allocating new memory, we definitely need it to save the path
    //I mean absolute path of the file.
    //the responsibility of free-ing up this memory lies on the calling method
    //as this reference is returned.

    f_path_tracker = malloc(sizeof(char) * PATH_MAX);
    usr_f_extension = f_extension;
    d_storg_path = f_dest_path;
    printf("f_dest_path: %s\n", f_dest_path);
    int code = nftw(root_path, f_callback_extsn, F_OPEN_LIMIT, FTW_PHYS);
    if(code == -1) {
        printf("\n%s ", msg_file_not_found);
        exit(CODE_ERROR_APP);
    }

    free(f_path_tracker);
    printf("\n\ndone with processing extension files.....\n");

    int is_tar = create_temp_tar_gz(d_storg_path);

    char* tar_path = malloc(sizeof(char) * (strlen(d_storg_path)+ strlen(name_tar)+1));
    if(is_tar == 0) {
        strcpy(tar_path, d_storg_path);
        strcat(tar_path, "/");
        strcat(tar_path, name_tar);
    } else {
        free(tar_path);
        return NULL;
    }

    return tar_path;
}

/*
 *  A utility function to copy file from one location to
 *  another location. The file descriptors are not closed
 *  in this function, and the responsibility lies on the
 *  calling method.
 */
int copy_file_fd(const int fd_src, const int fd_dest) {

    printf("Entering the copy file fd function\n");
    //creating a buffer, of size MAX_BUFFER_FILE_SIZE
    char* buffer = malloc(sizeof(char) * MAX_BUFFER_FILE_SIZE);

    //now, let's start the process of reading the data from the src file
    //read until there is nothing to be read or written.
    while(1) {
        long int rb = read(fd_src, buffer, MAX_BUFFER_FILE_SIZE);
        if(rb <=0) break;

        long int wb = write(fd_dest, buffer, MAX_BUFFER_FILE_SIZE);
        if(wb <= 0) break;
    }

    close(fd_src);
    close(fd_dest);
    free(buffer);
    return 0;
}

//open the file
int open_file(const char* f_path, const int oargs) {
    if(f_path == NULL) {
        printf("\n%s", " file path cannot be null.");
        return -1;
    }

    return open(f_path, oargs, 0777);
}

/**
 * Comparator to find the relatively larger timestamp.
 * @param x_time
 * @param y_time
 * @return
 */
int d_compare_modified(const void *x_time, const void *y_time) {
    const struct dentry *entry1 = (const struct dentry *)x_time;
    const struct dentry *entry2 = (const struct dentry *)y_time;

    return difftime(entry1->stat.st_mtimespec.tv_nsec, entry2->stat.st_mtimespec.tv_nsec);
}

/**
 * Comparator the names of the directories.
 * @param entry1
 * @param
 * @return
 */
int d_compare_name(const void *entry1, const void *entry2) {
    struct dentry *d_entry1 = (struct dentry *)entry1;
    struct dentry *d_entry2 = (struct dentry *)entry2;

//    printf("d_entry1->f_name: %s, d_entry2->f_name: %s\n", d_entry1->f_name, d_entry2->f_name);
    return strcmp(d_entry1->f_name, d_entry2->f_name);
}

DIR* open_dir(const char *d_path) {
    DIR  *_d = opendir(d_path);
    if(_d == NULL) {
        perror("error opening directory");
    }

    return _d;
}

int dir_count(char* d_path) {
    int i_count = 0;

    //try to open the directory entry
    DIR *_d = open_dir(d_path);
    if(_d != NULL) {
        struct dirent *temp = NULL;
        //count the number of directories for memory allocation
        while ((temp = readdir(_d)) != NULL) {
            if ((strcmp(temp->d_name, ".") == 0
                 || strcmp(temp->d_name, "..") == 0)) { continue; }

            if (temp->d_type == DT_DIR) {
                ++i_count;
            }
        }
    }
    //release resources
    closedir(_d);
    return i_count;
}

int is_linux() {
    struct utsname buffer;

    if (uname(&buffer) == -1) {
        perror("uname");
        return 1;
    }

    if (strcmp(buffer.sysname, "Darwin") == 0) {
        printf("This is a macOS system.\n");
        return 0;
    } else {
        return 1;
    }
}

/**
 * Utility function to substitute  path for home directory
 * represented by '~'.
 * @param command
 * @return
 */
char* expand_if_tilda(char *command) {

    if(command == NULL) {
        return NULL;
    }

    char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("getenv");
        exit(1);
    }

    size_t size = strlen(command) + strlen(home_dir)+1;
    char *exp_command = malloc(sizeof(char)*size);

    if(exp_command == NULL) {
        perror("malloc");
        exit(1);
    }

    size_t i = 0, j = 0;
    for(; command[i] != '\0'; i++, j++) {
        if(command[i] == C_TILDA) {
            strcpy(exp_command+j, home_dir);
            j += strlen(home_dir);
        } else {
            exp_command[j] = command[i];
        }
    }

    exp_command[i > j ? i : j] = C_NULL;
    return exp_command;
}
//todo: remover this
int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a; // Cast a to char* pointer
    const char *str2 = *(const char **)b; // Cast b to char* pointer

    printf("d_entry1->f_name: %s, d_entry2->f_name: %s\n", str1, str2);
    // Use strcmp for case-sensitive comparison
    return strcmp(str1, str2);
}


/**
 * Implementation of "dirlist" command, to get teh list of files from the directory @d_path.
 * The list is sorted based on the comparator pased as @sort_compare, using qsort library function.
 * @param d_path path to the root directory, where the
 * @param list the final list which contains list of names of subdirectories as part of @d_path
 * @param sort_compare pointer the function used for sorting the elements in the list
 * @return
 */
int list_dir_sort(char* d_path, struct dentry** list, int (*sort_compare)(const void*, const void*)) {

    //set default sorting comparator in case
    //none provided.
    if(sort_compare == NULL) {
        sort_compare = d_compare_name;
    }

    int i_count= dir_count(d_path);
    //try to open the directory entry
    if(i_count == 0) {
        return 0;
    }
    //initialize the list of dentry
    *list = (struct dentry*)malloc(sizeof (struct dentry) * i_count);
    DIR *_d = open_dir(d_path);
    // check if the _d directory is opened or not.
    if(_d != NULL) {
        i_count = 0; //reset counter
        struct dirent *ptr_file = NULL;
        printf("opened directory \n");

        //skip the local dir and parent directory.
        while((ptr_file = readdir(_d)) != NULL) {
            if((strcmp(ptr_file->d_name, ".") == 0
                || strcmp(ptr_file->d_name, "..") == 0)) { continue; }

            if(ptr_file->d_type == DT_DIR) {
                (*list)[i_count].f_name = strdup(ptr_file->d_name);
                    stat(ptr_file->d_name, &(*list)[i_count].stat);
                    ++i_count;
            }
        }

        //only for debugging purpose.
//        for(int i=0; i <i_count; i++) {
//            printf("%s\n", (*list)[i].f_name);
//        }
        //now as we have the list of all the directories present in the home directory
        //we need to sort the list by directory name in alphabetical order
        qsort(*list, i_count, sizeof(struct dentry), sort_compare);

        printf("total number of directories are ... %d\n", i_count);
        closedir(_d);
        return i_count;
    }

    closedir(_d);
    return 0;
}


int create_temp_tar_gz(const char *dirpath) {
    if (!dirpath) {
        return -1; // Error: filepath is NULL
    }

    // Construct the archive filename with full path based on dirpath
    char archive_name[PATH_MAX];
    snprintf(archive_name, PATH_MAX, "%s/%s", dirpath, name_tar);

    // Shell command to create the archive in the same directory
    int ret = snprintf(NULL, 0, "tar -czvf %s %s", archive_name, dirpath);
    if (ret < 0) {
        perror("snprintf");
        return -1;
    }

    char command[ret + 1];
    snprintf(command, ret + 1, "tar -czvf %s %s", archive_name, dirpath);

    // Execute the shell command using system
    ret = system(command);
    if (ret == -1) {
        perror("system");
        return -1;
    } else if (ret != 0) {
        // Handle non-zero return code from system (e.g., tar errors)
        fprintf(stderr, "Error: tar command failed with code %d\n", ret);
        return -1;
    }

    return 0;
}


