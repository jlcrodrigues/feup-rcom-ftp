#include "connection.h"

int openConnection(const char* address, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Opening socket failed.\n");
        exit(EXIT_FAILURE);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("Opening connection failed.\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int openControlConnection(Url url) {
    struct hostent *host;

    if ((host = gethostbyname(url.host)) == NULL) {
        perror("Retrieving host failed.\n");
        exit(EXIT_FAILURE);
    }

    const char* address = inet_ntoa(*((struct in_addr *) host->h_addr));

    int sockfd = openConnection(address, TCP_PORT);

    if (readCode(sockfd, "220") != 1) {
        perror("Opening connection failed: wrong status code.\n");
        exit(EXIT_FAILURE);
    }

    cleanSocket(sockfd);

    return sockfd;
}

int login(int sockfd, Url url) {
    char* buf = (char *)(malloc(BUFFER_SIZE));
    char* user = (char *)(malloc(BUFFER_SIZE));
    char* pass = (char *)(malloc(BUFFER_SIZE));
    sprintf(user, "user ");
    strcat(user, url.user);
    sprintf(pass, "pass ");
    strcat(pass, url.password);

    write(sockfd, user, strlen(user));
    write(sockfd, "\n", 1);
    if (readCode(sockfd, "331") == 0) {
        perror("Username invalid.\n");
        exit(EXIT_FAILURE);
    }

    write(sockfd, pass, strlen(pass));
    write(sockfd, "\n", 1);
    if (readCode(sockfd, "230") == 0) {
        perror("Password invalid.\n");
        exit(EXIT_FAILURE);
    }

    free(buf); free(user); free(pass);
    return 0;
}

int enterPassiveMode(int sockfd, char* address) {
    char* buf = (char *)(malloc(BUFFER_SIZE));
    char* code = (char *)(malloc(BUFFER_SIZE));

    // write pasv command
    const char* pasv = "pasv\n";
    write(sockfd, pasv, strlen(pasv));
    getSocketLine(sockfd, buf);
    memcpy(code, buf, 3);
    code[3] = '\0';
    if (strcmp(code, "227") != 0) {
        perror("Error entering passive mode.\n");
        exit(EXIT_FAILURE);
    }

    // parse given address and port.
    char *byte;
    byte = strtok(buf, "(");
    for (int i = 0; i < 4; i++) {
        byte = strtok(NULL, ",");
        strcat(address, byte);
        if (i < 3) strcat(address, ".");
    }
    byte = strtok(NULL, ",");
    int portMSB = atoi(byte);
    byte = strtok(NULL, ",");
    int portLSB = atoi(byte);

    return portMSB * 256 + portLSB;
}

void getFile(int sockfd, Url url, int datafd) {
    char* buf = (char *)(malloc(BUFFER_SIZE));
    const char retr[] = "retr ";

    write(sockfd, retr, strlen(retr));
    write(sockfd, url.path, strlen(url.path));
    write(sockfd, "\n", 1);

    FILE* file = fopen("file", "w");
    while (getSocketLine(datafd, buf) != -1) {
        fprintf(file, "%s", buf);
    }
}