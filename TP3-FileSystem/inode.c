#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1;
    }

    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int sector = INODE_START_SECTOR + (inumber - 1) / inodes_per_block;
    int offset = (inumber - 1) % inodes_per_block;

    char buffer[DISKIMG_SECTOR_SIZE];
    int ret = diskimg_readsector(fs->dfd, sector, buffer); 
    if (ret == -1) {
        return -1;
    }

    struct inode *inodes = (struct inode *) buffer;
    *inp = inodes[offset];

    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if (blockNum < 0) {
        return -1;
    }

    if (!(inp->i_mode & ILARG)) {
        if (blockNum >= 8) {
            return -1;
        }
        return inp->i_addr[blockNum];
    }

    int ptrs_per_block = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    int single_indirect_max = 7 * ptrs_per_block;

    if (blockNum < single_indirect_max) {
        int which_block = blockNum / ptrs_per_block;
        int offset = blockNum % ptrs_per_block;

        uint16_t indirect_block[ptrs_per_block];
        int ret = diskimg_readsector(fs->dfd, inp->i_addr[which_block], indirect_block);
        if (ret == -1) return -1;

        return indirect_block[offset];
    }

    int double_block_num = blockNum - single_indirect_max;
    int indirect_index = double_block_num / ptrs_per_block;
    int offset = double_block_num % ptrs_per_block;

    uint16_t double_indirect_block[ptrs_per_block];
    int ret = diskimg_readsector(fs->dfd, inp->i_addr[7], double_indirect_block); 
    if (ret == -1) return -1;

    if (indirect_index >= ptrs_per_block) return -1;

    uint16_t indirect_block[ptrs_per_block];
    ret = diskimg_readsector(fs->dfd, double_indirect_block[indirect_index], indirect_block);
    if (ret == -1) return -1;

    return indirect_block[offset];
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1); 
}
