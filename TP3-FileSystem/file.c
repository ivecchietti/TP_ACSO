#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode inode;
    if (inode_iget(fs, inumber, &inode) < 0) {
        return -1;
    }

    int disk_block = inode_indexlookup(fs, &inode, blockNum);
    if (disk_block == -1) {
        return -1;
    }

    int ret = diskimg_readsector(fs->dfd, disk_block, buf);
    if (ret == -1) {
        return -1;
    }

    int filesize = inode_getsize(&inode);
    int block_start = blockNum * DISKIMG_SECTOR_SIZE;
    int remaining = filesize - block_start;

    if (remaining >= DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    } else if (remaining > 0) {
        return remaining;
    } else {
        return -1; 
    }
}
