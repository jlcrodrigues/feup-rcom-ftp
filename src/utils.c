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