#include "utils.h"

int regGroupCopy(char* field, char* url_str, regmatch_t reg) {
    if (reg.rm_so == -1) return 1;
    int dist = reg.rm_eo - reg.rm_so;
    memcpy(field, &url_str[reg.rm_so], dist);
    field[dist] = '\0';
    return 0;
}

int parseUrl(char* url_str, Url* url) {
    const char *regex ="ftp://(([^/:].+):([^/:@].+)@)*([a-z.]+)/([a-z/]+)";
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
        url->user = "";
        url->password = "";
    }
    if (regGroupCopy(url->host, url_str, groups[4])) return 1;
    if (regGroupCopy(url->path, url_str, groups[5])) return 1;

    return 0;
}

int openConnection(Url url) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

    if ((host = gethostbyname(url.host)) == NULL) {
        perror("Retrieving host failed.\n");
        exit(EXIT_FAILURE);
    }

    const char* address = inet_ntoa(*((struct in_addr *) host->h_addr));

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(TCP_PORT);        /*server TCP port must be network byte ordered */

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