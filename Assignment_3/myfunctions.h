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

// helper functions

// just to keep track of the number of bytes read
int readbytes;

int openFs()
{
    int fd = open(fsimage, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }
    return fd;
}

inode_t findInode(int inum, int fd)
{
    // need to read the inode bitmap to check if the inode is vali
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

super_t readSuperBlock(int fd)
{
    super_t superBlock;
    readbytes = pread(fd, &superBlock, sizeof(super_t), 0);
    if (readbytes != sizeof(super_t))
    {
        perror("pread");
        exit(1);
    }
    return superBlock;
}

int getFreeBlock(int fd)
{
    // read super block
    super_t superBlock = readSuperBlock(fd);

    // read the data bitmap
    bitmap_t dataBitmap;
    readbytes = pread(fd, &dataBitmap, sizeof(bitmap_t), superBlock.data_bitmap_addr * UFS_BLOCK_SIZE);
    if (readbytes != sizeof(bitmap_t))
    {
        perror("pread");
        exit(1);
    }

    // find the first free block
    for (int i = 0; i < superBlock.num_data; i++)
    {
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(unsigned int); j++)
        {
            if (dataBitmap.bits[j] == 0)
            {
                dataBitmap.bits[j] = 0x1 << 31;
                readbytes = pwrite(fd, &dataBitmap, sizeof(bitmap_t), superBlock.data_bitmap_addr * UFS_BLOCK_SIZE);
                if (readbytes != sizeof(bitmap_t))
                {
                    perror("pwrite");
                    exit(1);
                }
                return i;
            }
        }
    }
    perror("No free block found");
}

int getFreeInode(int fd)
{
    // read super block
    super_t superBlock = readSuperBlock(fd);

    // read the inode bitmap
    bitmap_t inodeBitmap;
    readbytes = pread(fd, &inodeBitmap, sizeof(bitmap_t), superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE);
    if (readbytes != sizeof(bitmap_t))
    {
        perror("pread");
        exit(1);
    }

    // find the first free inode
    for (int i = 0; i < superBlock.num_inodes; i++)
    {
        for (int j = 0; j < UFS_BLOCK_SIZE / sizeof(unsigned int); j++)
        {
            if (inodeBitmap.bits[j] == 0)
            {
                inodeBitmap.bits[j] = 0x1 << 31;
                readbytes = pwrite(fd, &inodeBitmap, sizeof(bitmap_t), superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE);
                if (readbytes != sizeof(bitmap_t))
                {
                    perror("pwrite");
                    exit(1);
                }
                return i;
            }
        }
    }
    perror("No free inode found");
}

// main functions
int MFS_Lookup(int pinum, char *name)
{
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

int MFS_Stat(int inum, MFS_Stat_t *m)
{
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

int MFS_Write(int inum, char *buffer, int offset, int nbytes)
{
    // open the file
    int fd = openFs();

    // read and reaturn the inode
    inode_t fileInode = findInode(inum, fd);

    // read the super block
    super_t superBlock = readSuperBlock(fd);

    // number of blocks required
    int no_block_required = (strlen(buffer) + offset) / UFS_BLOCK_SIZE + ((strlen(buffer) + offset) % UFS_BLOCK_SIZE != 0);

    if (no_block_required > DIRECT_PTRS)
    {
        perror("File too large");
    }
    // write the file
    int offsetBlock = offset / UFS_BLOCK_SIZE;
    int offsetByteInBlock = offset % UFS_BLOCK_SIZE;
    // set offsetBlock to
    for (int i = 0; i < no_block_required; i++)
    {
        // if the block is not allocated
        if (fileInode.direct[i] == -1)
        {
            fileInode.direct[i] = getFreeBlock(fd);
        }
        if (offsetBlock != 0)
        {
            offsetBlock--;
            continue;
        }
        readbytes = pwrite(fd, buffer, UFS_BLOCK_SIZE, fileInode.direct[i] * UFS_BLOCK_SIZE + offsetByteInBlock);
        if (readbytes != UFS_BLOCK_SIZE)
        {
            perror("pwrite");
            exit(1);
        }
        if (strlen(buffer) < UFS_BLOCK_SIZE - offsetByteInBlock)
        {
            printf("all blocked writen");
            break;
        }
        // now the buffer moves to the next block to write
        *buffer == &buffer[UFS_BLOCK_SIZE - offsetByteInBlock];
        offsetByteInBlock = 0;
    }
    fileInode.size = max(no_block_required * UFS_BLOCK_SIZE, fileInode.size);
    // write the inode back
    readbytes = pwrite(fd, &fileInode, sizeof(inode_t), superBlock.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t));
    if (readbytes != sizeof(inode_t))
    {
        perror("pwrite");
        exit(1);
    }
    printf("writen inode back\n");

    close(fd);

    return 0;
}

int MFS_Read(int inum, char *buffer, int offset, int nbytes)
{
    char *head = buffer;
    // open the file
    int fd = openFs();

    // read and reaturn the inode
    inode_t fileInode = findInode(inum, fd);

    int offsetBlock = offset / UFS_BLOCK_SIZE;
    int offsetByteInBlock = offset % UFS_BLOCK_SIZE;

    for (int i = 0; i < DIRECT_PTRS; i++)
    {
        readbytes = 0;
        // if the block is not allocated
        if (fileInode.direct[i] == -1)
        {
            break;
        }
        if (offsetBlock != 0)
        {
            offsetBlock--;
            continue;
        }
        readbytes = pread(fd, buffer, min(nbytes, UFS_BLOCK_SIZE - offsetByteInBlock), fileInode.direct[i] * UFS_BLOCK_SIZE + offsetByteInBlock);
        if (readbytes != nbytes)
        {
            // if nbytes + offset is greater than the block size
            *buffer == &buffer[readbytes];
            nbytes -= readbytes;
            offsetByteInBlock = 0;
            continue;
        }
    }
    buffer = head;
    close(fd);

    return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{

    // open the file
    int fd = openFs();

    // read super block
    super_t superBlock = readSuperBlock(fd);

    // read and reaturn the parent inode
    inode_t parentInode = findInode(pinum, fd);

    // check if the parent is dir or not
    if (parentInode.type != MFS_DIRECTORY)
    {
        perror("Parent is not a directory");
        return -1;
    }

    // find a empty inode in indoebitmap
    int inum = getFreeInode(fd);

    inode_t fileInode;
    fileInode.type = type;
    // make a inode for file if file type
    if (type == MFS_REGULAR_FILE)
    {
        fileInode.size = 0;
        // empty file
        for (int i = 0; i < DIRECT_PTRS; i++)
            fileInode.direct[i] = -1;
    }
    if (type == MFS_DIRECTORY)
    {
        fileInode.size = 2 * sizeof(dir_ent_t);
        // empty dir will have "." "..", find one block for it
        fileInode.direct[0] = getFreeBlock(fd);
        dir_block_t dirBlock;
        strcpy(dirBlock.entries[0].name, ".");
        dirBlock.entries[0].inum = inum;

        strcpy(dirBlock.entries[1].name, "..");
        dirBlock.entries[1].inum = pinum;

        // rest -1
        for (int i = 2; i < 128; i++)
            dirBlock.entries[i].inum = -1;
        // write it on the block
        readbytes = pwrite(fd, &dirBlock, UFS_BLOCK_SIZE, fileInode.direct[0] * UFS_BLOCK_SIZE);
        if (readbytes != UFS_BLOCK_SIZE)
        {
            perror("pwrite");
            exit(1);
        }
        // rest empty direct pointers in inode
        for (int i = 1; i < DIRECT_PTRS; i++)
            fileInode.direct[i] = -1;
    }
    // write the inode back
    readbytes = pwrite(fd, &fileInode, sizeof(inode_t), superBlock.inode_region_addr * UFS_BLOCK_SIZE + inum * sizeof(inode_t));
    if (readbytes != sizeof(inode_t))
    {
        perror("pwrite");
        exit(1);
    }
    return 0;
}

int MFS_Unlink(int pinum, char *name)
{
    // open the file
    int fd = openFs();
    // read super block
    super_t superBlock = readSuperBlock(fd);
    // find the parent inode
    inode_t parentInode = findInode(pinum, fd);
    // check if dir or not
    if (parentInode.type != MFS_DIRECTORY)
    {
        perror("Parent is not a directory");
        return -1;
    }
    // find the name
    int inum = MFS_Lookup(pinum, name);
    if (inum == -1)
    {
        perror("not found");
        return -1;
    }
    // read the inum
    inode_t fileInode = findInode(inum, fd);
    if (fileInode.type == MFS_DIRECTORY)
    {
        if (fileInode.size > 2 * sizeof(dir_ent_t))
        {
            perror("Directory not empty");
            return -1;
        }
    }
    // itrate over all the direct pointers of the parentInode
    for (int i = 0; i < DIRECT_PTRS; i++)
    {
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
                    printf("deleting %s\n", name);
                    strcpy(dirBlock.entries[j].name, "");
                    dirBlock.entries[j].inum = -1;
                    // write the block back
                    readbytes = pwrite(fd, &dirBlock, sizeof(dir_block_t), parentInode.direct[i] * UFS_BLOCK_SIZE);
                    // mark the inode as free
                    bitmap_t inodeBitmap;
                    readbytes = pread(fd, &inodeBitmap, sizeof(bitmap_t), superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE);
                    inodeBitmap.bits[inum] = 0x0 << 31;
                    readbytes = pwrite(fd, &inodeBitmap, sizeof(bitmap_t), superBlock.inode_bitmap_addr * UFS_BLOCK_SIZE);
                    return 0;
                }
            }
            i++;
        }
    }
}

#endif // MYFUNCTIONS_H