#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connection.h"

int main(int argc, char *argv[]) {
    Url url;

    if (argc < 2) {
        printf("Usage:\n\t./download [file_path]\n");
        exit(EXIT_FAILURE);
    }

    if (parseUrl(argv[1], &url)) {
        printf("The provided url is not valid.\n");
        exit(EXIT_FAILURE);
    }

    // open a tcp connection at the given url
    int sockfd = openControlConnection(url);

    login(sockfd, url);

    // create and connect to a tcp connection in passive mode
    char* address = (char *)(malloc(BUFFER_SIZE));
    address[0] = '\0';
    int port = enterPassiveMode(sockfd, address);
    int datafd = openConnection(address, port);

    getFile(sockfd, url, datafd);

    close(sockfd);
    close(datafd);

    return 0;
}