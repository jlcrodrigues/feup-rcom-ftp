#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 256
#define MAX_GROUPS 6

typedef struct Url {
    char* user; 
    char* password; 
    char* host;
    char* path;
} Url;

/**
 * @brief Fill a Url field from a regex result.
 * 
 * @param field Destination.
 * @param url_str Original url string.
 * @param reg Regex result.
 * @return int Returns 0 upon success, 1 otherwise.
 */
int regGroupCopy(char* field, char* url_str, regmatch_t reg);

/**
 * @brief Parse a url and store the individual fields.
 * 
 * @param url_str Original url string.
 * @param url Url struct to store different fields.
 * @return int Returns 0 upon success, 1 otherwise.
 */
int parseUrl(char* url_str, Url* url);