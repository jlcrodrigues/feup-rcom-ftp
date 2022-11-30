#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

int main() {
    Url url;
    if (parseUrl("ftp://usr:pwd@ftp.up.pt/pub/", &url)) {
        printf("The provided url is not valid.\n");
        return 1;
    }
    printf("user - %s\n", url.user);
    printf("password - %s\n", url.password);
    printf("host - %s\n", url.host);
    printf("path - %s\n", url.path);
    return 0;
}