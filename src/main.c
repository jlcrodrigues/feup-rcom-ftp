#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"

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

    int sockfd = openConnection(url);

    return 0;
}