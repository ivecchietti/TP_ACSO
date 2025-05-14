#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt) {
    struct inode dir_inode;
    if (inode_iget(fs, dirinumber, &dir_inode) < 0) {
        return -1;
    }

    int size = inode_getsize(&dir_inode);
    int num_entries = size / sizeof(struct direntv6);

    char blockbuf[DISKIMG_SECTOR_SIZE];

    for (int i = 0; i * sizeof(struct direntv6) < size; ++i) {
        int blockNum = (i * sizeof(struct direntv6)) / DISKIMG_SECTOR_SIZE;
        int offsetInBlock = (i * sizeof(struct direntv6)) % DISKIMG_SECTOR_SIZE;

        if (offsetInBlock == 0) {
            if (file_getblock(fs, dirinumber, blockNum, blockbuf) < 0) {
                return -1;
            }
        }

        struct direntv6 *entry = (struct direntv6 *) (blockbuf + offsetInBlock);

        if (strncmp(name, entry->d_name, 14) == 0) {
            *dirEnt = *entry;
            return 0;
        }
    }
    return -1; 
}
