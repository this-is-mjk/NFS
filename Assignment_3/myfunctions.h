#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#define fsimage "/Users/this_is_mjk/Projects/Learning/SummerProject24/Pclub/NFS/Assignment_3/fsimage"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ufs.h"
#include "mfs.h"

// just to keep track of the number of bytes read
int readbytes;

int openFs(){
    int fd = open(fsimage, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    return fd;
}

inode_t findInode(int inum, int fd) {

    // read super block
    // read super block
    super_t superBlock;
    readbytes = pread(fd, &superBlock, sizeof(super_t), 0);
    if (readbytes != sizeof(super_t))
    {
        perror("pread");
        exit(1);
    }
    printf("inode_region_addr %d\n", superBlock.inode_region_addr);

    // read the parentInode
    inode_t parentInode;
    readbytes = pread(fd, &parentInode, sizeof(inode_t), superBlock.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t));
    printf("parentInode.type %d\nparentInode.size %d\n", parentInode.type, parentInode.size);
    return parentInode;
}

// takes the parent inode number and looks up the entry name in it
int MFS_Lookup(int pinum, char *name) {
    // open the file
    int fd = openFs();

    // read and reaturn the parent inode
    inode_t parentInode = findInode(pinum, fd);

    // itrate over all the direct pointers of the parentInode
    for (int i = 0; i < DIRECT_PTRS; i++)
    {
        printf("parentInode.direct[%d] %d\n", i, parentInode.direct[i]);
        if (parentInode.direct[i] != -1)
        {
            dir_block_t dirBlock;
            readbytes = pread(fd, &dirBlock, sizeof(dir_block_t), parentInode.direct[i] * UFS_BLOCK_SIZE);
            if (readbytes != sizeof(dir_block_t))
            {
                perror("pread");
                exit(1);
            }
            // itrate over all the entries of the dirBlock of a single direct pointer
            for (int j = 0; j < 128; j++)
            {
                // is found return the inode number
                if (strcmp(dirBlock.entries[j].name, name) == 0)
                {
                    printf("%s\n", name);
                    return dirBlock.entries[j].inum;
                }
                // if -1 starts then all the entries are over
                if (dirBlock.entries[j].inum == -1)
                {
                    // not found
                    return -1;
                }
            }
            i++;
        }
        // if -1 then all the pointers are over
        else
        {
            // not found
            return -1;
        }
    }

    close(fd);
    return 0;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {
    // open the file
    int fd = openFs();

    // read and reaturn the parent inode
    inode_t parentInode = findInode(inum, fd);

    // if not valid inode size is 0
    if (parentInode.size == 0)
    {
        return -1;
    }
    // write to the MFS_Stat_t struct
    m->size = parentInode.size;
    m->type = parentInode.type;

    close(fd);

    return 0;
}

int MFS_Write(int inum, char *buffer, int offset, int nbytes){
    // open the file
    int fd = openFs();

    // read and reaturn the parent inode
    inode_t fileInode = findInode(inum, fd);

    // number of blocks required
    int no_block_required = (strlen(buffer) + offset)/UFS_BLOCK_SIZE + ((strlen(buffer) + offset)%UFS_BLOCK_SIZE != 0);

    if (no_block_required > DIRECT_PTRS){
        perror("File too large");
    }
    int bitWriten = 0;
    // write the file
    int offsetBlock = offset/UFS_BLOCK_SIZE;

    close(fd);

    return 0;
}

#endif // MYFUNCTIONS_H