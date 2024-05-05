//
// Created by Anuj Puri on 2024-03-28.
//

#ifndef FILE_DOWNLOAD_SERVER_STRINNGUTILS_H
#define FILE_DOWNLOAD_SERVER_STRINNGUTILS_H

#define MAX_STR_TOKENS 20

#endif //FILE_DOWNLOAD_SERVER_STRINNGUTILS_H


#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

//constant literals
const char C_SPACE = ' ';
const char C_PERIOD = '.';

const char C_NULL = '\0';
const char C_TILDA = '~';

const char* SYMBOL_FWD_SLASH = "/";
const char* STR_SPACE = " ";
const char* C_NEW_LINE = "\n";

//string returns
char* trim(const char* str);
int extcmp(const char* f_name, char* ext);

//no-value returning functions
void recycle_str(char* ptr_str, unsigned long pos);


/**
 * Utility function to remove the leading and trailing space
 * in the provided string.
 * @param str string
 * @return
 *
 */
char* trim(const char* str) {
    char* begin_str = str;
    if(*begin_str == '\0') {
        return begin_str;
    }

    //find the first non-space character,
    while(*begin_str == C_SPACE) {
        begin_str++;
    }

    //if found null, return str
    if(*begin_str == '\0') {
        return begin_str;
    }

    char* from_end = begin_str + strlen(begin_str) - 1;
    while(from_end > begin_str && *from_end == C_SPACE) {
        from_end--;
    }

    *(from_end+1) = '\0';

    return begin_str;
}


/**
 * Check if the file contains the required extension.
 * @param f_name name of the file
 * @param ext extension to be compared.
 * @return 0 if the files as extenstion @ext, else > 0, and -1 in case any of the arg is NULL.
 */
int extcmp(const char* f_name, char* ext) {

    if(f_name == NULL || ext == NULL) {
        return -1;
    }

    size_t filename_len = strlen(f_name);

    // Ensure filename is at least 5 characters long (including ".txt")
    if (filename_len < 5) {
        return -1;  // Not a .txt file
    }

    size_t idx_period = strlen(f_name)-1;
    for(;idx_period >= 0; idx_period--) {
        if(f_name[idx_period] == C_PERIOD) {
            break;
        }
    }
    //comparing the characters starting from idx_period, include period symbol.
    return strcmp(f_name + idx_period, ext);
}

//A utility created to re-use the char* (string)
//rather than dynamically allocating memory again and again.
//the idea is similar to how StringBuilder works in Java
void recycle_str(char* ptr_str, unsigned long pos) {
    if(ptr_str == NULL || (pos >= strlen(ptr_str))) {
        return;
    }

    for(unsigned long i=pos; i < strlen(ptr_str)-1; i++) {
        *(ptr_str+i) = '\0';
    }

}

/**
 * Tokenize the provided string using delimiter.
 * The function does not modify the existing "str" provided.
 * @param str string to be tokenized.
 * @param delim delimiter value.
 * @return a vector of tokens, each row contains one token.
 */
char** tokenize(char* str, char delim, int* count) {

    char* temp = malloc(sizeof(char) * strlen(str)+1);
    strcpy(temp, str);

    char* token = NULL;
    token = strtok(temp, &delim);
    int num_tokens = 0;
    while(token != NULL) {
        num_tokens++;
        token = strtok(NULL,  &delim);
    }
    *count = num_tokens;
    free(temp);

    char** tokens = NULL;
    tokens = malloc(sizeof(char *) * num_tokens);
    temp = malloc(sizeof(char) * strlen(str)+1);
    strcpy(temp, str);
    token = strtok(temp, &delim);
    printf(" toke is %s\n", token);
    num_tokens = 0;

    while(token != NULL) {
        tokens[num_tokens] = malloc(strlen(token)+1);
        strcpy(tokens[num_tokens++], token);
        token = strtok(NULL,  &delim);
    }

    free(temp);
    return tokens;
}

/**
 * Utility function to return the permissions in human readable form.
 * @param mode
 * @return
 */
char *get_permissions(mode_t mode) {
    static char permissions[11]; // Buffer to store permissions string

    // Initialize permissions string with dashes (-)
    memset(permissions, '-', 10);
    permissions[10] = '\0'; // Null terminate the string

    // Check owner permissions
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_IXUSR) ? (mode & S_ISUID) ? 's' : 'x' : '-';

    // Check group permissions
    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_IXGRP) ? (mode & S_ISGID) ? 's' : 'x' : '-';

    // Check other permissions
    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_IXOTH) ? (mode & S_ISVTX) ? 't' : 'x' : '-';

    // Add special permission characters
    permissions[9] = (mode & S_ISUID) ? 'S' : (mode & S_ISGID) ? 'S' : (mode & S_ISVTX) ? 'T' : '-';

    return permissions;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

char *ulong_to_string(unsigned long value) {
    int num_digits = 0;
    unsigned long temp = value;

    // Count the number of digits
    do {
        temp /= 10;
        num_digits++;
    } while (temp > 0);

    // Allocate memory for the string (including null terminator)
    int alloc_size = num_digits + 1;
    char *result = (char*)malloc(alloc_size * sizeof(char));
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    // Convert digits to characters and store in the string (reverse order)
    temp = value;
    result[num_digits] = '\0'; // Initialize null terminator
    while (temp > 0) {
        int digit = temp % 10;
        result[num_digits-- - 1] = digit + '0';
        temp /= 10;
    }

    return result;
}

int is_number(char* str) {

    if(str == NULL) {
        return 0;
    }

    for(int i = 0; i < strlen(str); i++) {
        if(!isdigit(str[i])) {
            return 0;
        }
    }

    return 1;
}