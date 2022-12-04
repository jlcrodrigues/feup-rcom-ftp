#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define MAX_GROUPS 6
#define TCP_PORT 21

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

/**
 * @brief Creates a socket and connects to the server specified in url.
 * 
 * @return int Returns the socket's file descriptor.
 */
int openConnection(const char* address, int port);

/**
 * @brief Creates a connection to be used for control.
 * 
 * @param url Url struct containing the url of the connection to be opened.
 * @return int Returns the socket's file descriptor.
 */
int openControlConnection(Url url);

/**
 * @brief Read a line from a socket.
 * 
 * @param socket Socket file descriptor.
 * @param buf Pointer to store the line read.
 * @return Returns 0 upon success, -1 otherwise.
 */
int getSocketLine(int sockfd, char* line);

/**
 * @brief Verify is the connection return the right code.
 * 
 * @param socket Socket file descriptor.
 * @param expected Expected status code, e.g. "220".
 * @return int 1 if the code is correct, -1 for error, and 0 otherwise
 */
int readCode(int sockfd, char* expected);

/**
 * @brief Read a socket until the end.
 * 
 * @param socket Socket file descriptor.
 */
void cleanSocket(int sockfd);

/**
 * @brief Login into the tcp connection.
 * 
 * @param sockfd 
 * @param url Previously parsed url that contains the credentials (or not).
 * @return int Returns 0 on success.
 */
int login(int sockfd, Url url);

/**
 * @brief Enter passive mode.
 * 
 * @param sockfd Socket file descriptor.
 * @param address String to be filled with the address given by the server.
 * @return int Port number where server awaits connection.
 */
int enterPassiveMode(int sockfd, char* address);

void getFile(int sockfd, Url url);