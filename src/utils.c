#include "utils.h"

int regGroupCopy(char* field, char* url_str, regmatch_t reg) {
    if (reg.rm_so == -1) return 1;
    int dist = reg.rm_eo - reg.rm_so;
    memcpy(field, &url_str[reg.rm_so], dist);
    field[dist] = '\0';
    return 0;
}

int parseUrl(char* url_str, Url* url) {
    const char *regex ="ftp://(([^/:].+):([^/:@].+)@)*([a-z.]+)/([a-z/.]+)";
    url->user = malloc(BUFFER_SIZE);
    url->password = malloc(BUFFER_SIZE);
    url->host = malloc(BUFFER_SIZE);
    url->path = malloc(BUFFER_SIZE);
  
    regex_t regex_comp;
    regmatch_t groups[MAX_GROUPS];

    if (regcomp(&regex_comp, regex, REG_EXTENDED)) {
        return 1;
    };

    if (regexec(&regex_comp, url_str, MAX_GROUPS, groups, 0)) {
        return 1;
    }

    if (groups[2].rm_so != -1) { // user and password optional
        if (regGroupCopy(url->user, url_str, groups[2])) return 1;
        if (regGroupCopy(url->password, url_str, groups[3])) return 1;
    }
    else {
        url->user = "anonymous";
        url->password = "";
    }
    if (regGroupCopy(url->host, url_str, groups[4])) return 1;
    if (regGroupCopy(url->path, url_str, groups[5])) return 1;

    return 0;
}

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

int getSocketLine(int sockfd, char* line) {
    FILE* socket = fdopen(sockfd, "r");
    char* buf = (char*)(malloc(BUFFER_SIZE));
    size_t size = 0;
    int nbytes;
    if (!((nbytes = getline(&buf, &size, socket)) >= 0)) 
        return -1;
    memcpy(line, buf, nbytes);
    free(buf);
    return 0;
}

int readCode(int sockfd, char* expected) {
    char* buf = (char*)(malloc(BUFFER_SIZE));
    if (getSocketLine(sockfd, buf) != 0) {
        perror("Opening connection failed.\n");
        exit(EXIT_FAILURE);
    }
    buf[3] = '\0';
    int res = (strcmp(buf, expected) == 0);
    free(buf);
    return res;
}

void cleanSocket(int sockfd) {
    FILE* socket = fdopen(sockfd, "r");
    char* buf = (char*)(malloc(BUFFER_SIZE));
    size_t size = 0;
    while (getline(&buf, &size, socket) >= 0) {
        // read is over when '-' is missing after the status code
        if (buf[3] != '-') break;
    }
    free(buf);
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
