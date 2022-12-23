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

    cleanSocket(sockfd);

    write(sockfd, user, strlen(user));
    write(sockfd, "\n", 1);

    /* read login code */
    char* code = (char*)(malloc(BUFFER_SIZE));
    if (getSocketLine(sockfd, code) != 0) {
        perror("Reading login failed.\n");
        exit(EXIT_FAILURE);
    }
    code[3] = '\0';

    if (strcmp(code, "331") != 0 && strcmp(code, "230") != 0) {
        perror("Username invalid.\n");
        exit(EXIT_FAILURE);
    }
    else if (strcmp(code, "331") == 0) {
        write(sockfd, pass, strlen(pass));
        write(sockfd, "\n", 1);
        if (readCode(sockfd, "230") == 0) {
            perror("Password invalid.\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(code, "230") == 0) {
        cleanSocket(sockfd);
        getSocketLine(sockfd, buf);
    }

    free(buf); free(user); free(pass); free(code);
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
    int file, nbytes;
    char* buf = (char *)(malloc(1024));
    const char retr[] = "retr ";

    write(sockfd, retr, strlen(retr));
    write(sockfd, url.path, strlen(url.path));
    write(sockfd, "\n", 1);

    if ((file = open(getFileName(url.path), O_WRONLY | O_CREAT, 0777)) == -1) {
        perror("Error creating the new file.\n");
        exit(EXIT_FAILURE);
    }

    while ((nbytes = read(datafd, buf, 1024)) != 0) {
        write(file, buf, nbytes);
    }
}