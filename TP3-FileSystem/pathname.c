
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (!pathname || pathname[0] != '/') {
        return -1;  
    }
    
    if (strcmp(pathname, "/") == 0) {
        return 1;
    }

    int curr_inumber = 1;

    char path_copy[1024];
    strncpy(path_copy, pathname, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char *token = strtok(path_copy, "/");

    while (token != NULL) {
        struct direntv6 entry;

        if (directory_findname(fs, token, curr_inumber, &entry) < 0) {
            return -1;
        }

        curr_inumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return curr_inumber;
}
