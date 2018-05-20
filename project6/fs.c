// Operating System Principles
// Project 6
// Professor Thain
// 5/3/2017
// Aron Lam

// INCLUDED FILES //
#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

// STRUCTS & UNION //
struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int direct[POINTERS_PER_INODE];
	int indirect;
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

// GLOBALS //
// Total Inodes in the Filesystem, 
int totalInodes; 

// Two 1-dimenstional bitmaps, 1 for free inodes, 1 for free data blocks, the size of bitmaps depends on the data from the superblock in fs_mount
int *fbbitmap;
int bitmapSize;

// Populate the inode bitmap during mount by scanning all the inode blocks and then seeing which ones have the isvalid flag set
int *inodeMap; 

// Flag, global variable that tells us whether fs is mounted or not 
int mounted = 0;


// HELPER FUNCTIONS
// load data from inode at inumber
void inode_load(int inumber, struct fs_inode *inode) {
    union fs_block block;
    disk_read((int)(inumber / INODES_PER_BLOCK) + 1, block.data);

    // use a computation to transform an inumber to a block number
    *inode = block.inode[inumber % INODES_PER_BLOCK];
}

// save data from inode at inumber into passed inode
void inode_save(int inumber, struct fs_inode *inode) {
    union fs_block block;
    disk_read((int) (inumber / INODES_PER_BLOCK) + 1, block.data);

    // use a computation to transform an inumber to a block number
    block.inode[inumber % INODES_PER_BLOCK] = *inode;
    disk_write((int)(inumber/INODES_PER_BLOCK) + 1, block.data);
}


// FS FUNCTIONS //
// Creates a new filesystem on the disk, destroying any data already present. Sets aside 10% of the blocks for inodes, clears the inode table, and writes the superblock. Returns 1 on success, 0 otherwise. *Formatting a filesystem does not cause it to be mounted. 
// format should do minimum work to create a blank fs: valid superblock and no valid inodes
int fs_format() {
    union fs_block block;
    disk_read(0, block.data);

    // if user tries to format a disk that is already mounted, return failure
    if (mounted) {
    	return 0;
    }
    
    // initialize new superblock
    block.super.magic = FS_MAGIC;
        // set nblocks to disk.size()
    block.super.nblocks = disk_size();
        // set aside 10% of blocks for inodes
    block.super.ninodeblocks = (int)((disk_size() + 9)/10); 
    block.super.ninodes = block.super.ninodeblocks*INODES_PER_BLOCK;

    // write superblock
    disk_write(0, block.data);
    
    // Allocate memory for inode bitmap
    inodeMap = malloc(block.super.ninodeblocks*INODES_PER_BLOCK*sizeof(int));
    
    // to clear out the inode Table, blitz out the number of inode blocks (zeroing them out to essentilly a blank inode data structure)
    int i, j, k;
    for (i = 1; i <= block.super.ninodeblocks; i++) {
    	disk_read(i, block.data);
    	for (j = 0; j < INODES_PER_BLOCK; j++) {
    		block.inode[j].isvalid = 0;
    		block.inode[j].size = 0;
            for (k = 0; k < POINTERS_PER_INODE; k++) {
                block.inode[j].direct[k] = 0;
            }
    		block.inode[j].indirect = 0;
    	}

    	// make sure to write those newly formatted inodes to disk
    	disk_write(i, block.data);
    }

    // on success
	return 1;
}

// Scan a mounted filesystem and report on how the inodes and blocks are organized.
void fs_debug() {
	union fs_block block;

    // read superblock
	disk_read(0, block.data);
    
    // Print statements to view properties
	printf("superblock:\n");
	if(block.super.magic == FS_MAGIC) {
	    printf("    magic number is valid\n");
	} else {
	    printf("    magic number is invalid\n");
	    //exit(1); ?
	}
	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d blocks for inodes\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);
	
	// scan a mounted filesystem and report how the inodes are organized
    int k, i, j, l;
	for (k = 1; k < block.super.ninodeblocks; k++) {
	    for (i = 0; i < INODES_PER_BLOCK; i++) {
	        // perform disk read of each inode block.
	        disk_read(k, block.data);
	        // Check inode validity
	        if (block.inode[i].isvalid) {
	            printf("inode %d:\n", i);
	            printf("    size %d bytes\n", block.inode[i].size);
	            
	            printf("    direct blocks: ");
	            // Print direct blocks
	            for(j = 0; j < POINTERS_PER_INODE; j++) {
	                if ( block.inode[i].direct[j] ) {
	                    printf("%d ", block.inode[i].direct[j]);
	                }  
	            }
	            printf("\n");

	            // Check if there are indirect blocks, if so print them out as well
	            if (block.inode[i].indirect) {
	                printf("    indirect block: %d\n", block.inode[i].indirect);
	                printf("    indirect data blocks: ");
	                disk_read(block.inode[i].indirect, block.data);
	                // print out the 'pointer' numbers directly in the slots;
	                //(pointers are not real C-style pointers, just ints)
	                for(l = 0; l < POINTERS_PER_BLOCK; l++) {
	                    if (block.pointers[l]) {
	                        printf("%d ", block.pointers[l]);
	                    }
	                }
	                printf("\n");
	        
	            }
	        }
	    }
	}
}

// Examine the disk for a filesystem. If one is present, read the superblock, build a free block bitmap, and prepare the filesystem for use. Return 1 on success, zero otherwise. A successful mount is a pre-requisite for the remaining calls.
// The superblock tells you how to mount, not if it has been mounted
int fs_mount() {
	union fs_block block;

    // if attempt to mount an already mounted fs, return error
    if (mounted) {
        return 0;
    }

    // read superblock
	disk_read(0, block.data);

    // creates the appropriate bitmaps denoting which storage is consumed. 
    bitmapSize = block.super.nblocks;
    // Allocate memory for free block bitmap
    fbbitmap = malloc(bitmapSize*sizeof(int));

    // variables for iteration
    int i, j, k, l, m, n;
    // Build free data block map 
    for (i = 0; i < bitmapSize; i++) {
        // mark as all empty
    	fbbitmap[i] = 0;
    }
    // Account for superblock in use
    fbbitmap[0] = 1;

    // Build inodeMap by traversng inodes
    totalInodes = (block.super.ninodeblocks*INODES_PER_BLOCK);
    // Allocate memory for inode bitmap
    inodeMap = malloc(totalInodes*sizeof(int));
    for (j = 0; j < totalInodes; j++) {
    	inodeMap[j] = 0;
    }

    // set up the free block bitmap for valid inodes and corresponding blocks
    for (k = 1; k <= block.super.ninodeblocks; k++) {
        // mark as taken
    	fbbitmap[k] = 1;
    	for (l = 0; l < INODES_PER_BLOCK; l++) {
    		disk_read(k, block.data);
    		// Check if inode validity
    		if (block.inode[l].isvalid) {
    			inodeMap[((k-1) * INODES_PER_BLOCK) + l] = 1;

    			// direct blocks in valid inodes
    			for (m = 0; m < POINTERS_PER_INODE; m++) {
    				if(block.inode[l].direct[m]) {
    					fbbitmap[block.inode[l].direct[m]] = 1;
    				}
    			}

    			// indirect blocks in valid inodes 
    			if (block.inode[l].indirect) {
    				fbbitmap[block.inode[l].indirect] = 1;
    				disk_read(block.inode[l].indirect, block.data);
                    // set bits as taken in free block bitmap if necessary
    				for (n = 0; n < POINTERS_PER_BLOCK; n++) {
    					if (block.pointers[n]) {
    						fbbitmap[block.pointers[n]] = 1;
    					}
    				}
    			}
    		}
    	}
    }
    // set mounted and return on success
    mounted = 1;
    return 1;
}

// Create a new inode of 0 length. On success, return the positive inumber. On failure return 0.
int fs_create() {
    struct fs_inode inode;

    // if unmounted
    if(mounted == 0) {
    	printf("fs is not mounted\n");
    	return 0;
    }

    // search for empty inode, once we find a free inode from the bitmap, set the size/isvalid and do a disk_write (IF Doing a copy in)
    int i, j;
    // create a file with the superblock and inode schema but with everything invalid and empty
    for (i = 1; i < totalInodes; i++) {
    	if (inodeMap[i] == 0) {
    		inodeMap[i] = 1;
    		inode_load(i, &inode);

            // Prepare inode for use
    		inode.isvalid = 1;
    		inode.size = 0;
    		for (j = 0; j < POINTERS_PER_INODE; j++) {
    			inode.direct[j] = 0;
    		}
    		inode.indirect = 0;
            // update inode
    		inode_save(i, &inode);

    		// on success, return positive inumber
    		return i;
    	}
    }
	// on failure
	return 0;
}

// delete the inode indicated by the inumber. Release all data and indirect blocks assigned to this inode and return them to the free block map. On success return 1, on failure return 0.
int fs_delete( int inumber ) {
	union fs_block block;
	struct fs_inode inode;

	// check if mounted
	if (mounted == 0) { // on failure
		printf("fs not mounted\n");
		return 0;
	}

	// read superblock
	disk_read(0, block.data);

	// a delete on an invalid inode should return failure
	if (inumber < 0 || inumber > block.super.ninodes) {
		return 0;
	}

	// load inode with inumber
	inode_load(inumber, &inode);

    // no need to delete an invalid inode
	if (inode.isvalid == 0) {
		return 1;
	} else {
		// if an inode is valid, then it cannot be used until the file it is 'containing' is deleted
		inode.isvalid = 0;
		inode.size = 0;

        // update the inode
		inode_save(inumber, &inode);

		// release all data and direct blocks assigned to this inode and return them to the free block map
		int i, j;
	    for (i = 0; i < POINTERS_PER_INODE; i++) {
	    	if (inode.direct[i]) {
	    		fbbitmap[inode.direct[i]] = 0;
	    	}
	    }

	    // release all data and indirect blocks assigned to this inode and return them to the free block map 
	    if (inode.indirect) {
	    	disk_read(inode.indirect, block.data);
	    	for (j = 0; j < POINTERS_PER_BLOCK; j++) {
	    		if (block.pointers[j]) {
	    			fbbitmap[block.pointers[j]] = 0;
	    		}
	    	}
	    }

	    // on success return 1
	    return 1; 
	}
}

// Return the logical size of the given inode in bytes. *0 is a valid logical size. On failure, return -1
int fs_getsize( int inumber ) {
	union fs_block block;
	struct fs_inode inode;

    // perform mount check
	if (mounted == 0) {
		printf("fs not mounted\n");
		return 0;
	}

    // read superblock
	disk_read(0, block.data);

	// getsize of invalid inode should return failure
	if (inumber < 0 || inumber > block.super.ninodes) {
		return -1;
	}

	// load inode w/ inumber
	inode_load(inumber, &inode);
	if (inode.isvalid) {
		// on success
		return inode.size;
	}

	// on failure
	return -1;
}

// Read data from valid inode. Copy "length" bytes from the inode into the "data" pointer, starting at "offset" in the inode. Return the total number of bytes read. The number of bytes actually read could be smaller than the number of bytes requested, perhaps if the end of the inode is reached. If the given inumber is invalid or any other error is encountered, return 0.
int fs_read( int inumber, char *data, int length, int offset ) {
	union fs_block block;
    union fs_block iBlock;
	struct fs_inode inode;

    // perform mount check
	if (mounted == 0) {
		printf("fs not mounted\n");
		return 0;
	}

    // read superblock
	disk_read(0, block.data);

	// reading an invalid inode should return failure
	if (inumber < 0 || inumber > block.super.ninodes) {
		return 0;
	}

	// load inode w/ inumber 
	inode_load(inumber, &inode);
    // perform checks on inode & cannot read from invalid inode or position greater than inode.size
    if (inode.isvalid == 0 || if inode.size <= offset) {
        return 0;
    }

    // Variables to keep track of total bytes read
    int begin = (int)(offset/DISK_BLOCK_SIZE);
    int byte = 0;
    int actualOffset = offset % 4096;
    int initial = 0;

    int i, j, k;
    for (i = begin; i < POINTERS_PER_INODE; i++) {
    	if (inode.direct[i]) {
            // if not initial read
    		if (initial == 0) {
    			disk_read(inode.direct[i], block.data);
                // work through bytes and read out to data
    			for (j = 0; j + actualOffset < DISK_BLOCK_SIZE; j++) {
    				if (block.data[j + actualOffset]) {
    					data[byte] = block.data[j + actualOffset];
    					byte++;
    					if (byte + offset >= inode.size) {
    						return byte;
    					}
    				} else {
    					return byte;
    				}
                    // if length is reached in terms of bytes read
    				if (byte == length) {
    					return length;
    				}
    			}

    			initial = 1;
    		} else { // initial is true
                // read from direct
    			disk_read(inode.direct[i], block.data);
    			for (k = 0; k < DISK_BLOCK_SIZE; k++) {
    				if (block.data[k]) {
    					data[byte] = block.data[k];
    					byte++;
    					if (byte + offset >= inode.size) {
    						return byte;
    					}
    				} else {
    					return byte;
    				}

    				if (byte == length) {
    					return length;
    				}
    			}
    		}
    	}
    }

    // traverse Indirect blocks
    if (inode.indirect) {
        // read from iBlock
    	disk_read(inode.indirect, iBlock.data);

        // read bytes from Indirect blocks
        int l, m;
        int iBegin = begin - 5;
    	for (l = iBegin; l < POINTERS_PER_BLOCK; l++) {
    		if (iBlock.pointers[l]) {
    			disk_read(iBlock.pointers[l], block.data);
                // read out bytes 
    			for (m = 0; m < DISK_BLOCK_SIZE; m++) {
    				if (block.data[m]) {
    					data[byte] = block.data[m];
    					byte++;
                        // cannot read more bytes than the inode size
    					if (byte + offset >= inode.size) {
    						return byte;
    					}
    				} else {
    					return byte;
    				}
                    // Return total bytes read if length is reached
    				if (byte == length) {
    					return length;
    				}
    			}
    		}
    	}
    }

    // Return total bytes read
	return byte;
}

// Write to valid inode: Copy "length" bytes from the pointer "data" into the inode starting at "offset" bytes. Allocate any necessary direct and indirect blocks in the process. Return the number of bytes actually written. The number of bytes actually written could be smaller than the number of bytes requested, perhaps if the disk becomes full. If the given inumber is invalid, or if any other error is encountered, return 0.
int fs_write( int inumber, const char *data, int length, int offset ) {
    // when you write a small file to an inode that is mostly filled, overwrite what is needed and keep the other content.
    union fs_block block;
    union fs_block iBlock;
	struct fs_inode inode;

    // check mount
	if (mounted == 0) {
		printf("fs not mounted\n");
		return 0;
	}

	// read superblock
	disk_read(0, block.data);

	// reading an invalid inode should return failure
	if (inumber < 0 || inumber > block.super.ninodes) {
		return 0;
	}

	// load inode w/ inumber
	inode_load(inumber, &inode);

    // if given inode with inumber is invalid return failure
	if (inode.isvalid == 0) {
        return 0;
    }

    // iterating variables 
    int i, j, k, l, m, n;

    // if inode not empty
    if (inode.size > 0) {
    	for (i = 0; i < POINTERS_PER_INODE; i++) {
    		if (inode.direct[i]) {
                // mark as not free
    			fbbitmap[inode.direct[i]] = 0;
    		}
    	}
    	// check for indirect block
    	if (inode.indirect) {
    		disk_read(inode.indirect, block.data);
    		fbbitmap[inode.indirect] = 0;
    		for (j = 0; j < POINTERS_PER_BLOCK; j++) {
    			if (block.pointers[j]) {
    				fbbitmap[block.pointers[j]] = 0;
    			}
    		}
    	}
    } 

    // Keep track of byte number written
    int byte = 0;
    int begin = (int)(offset/DISK_BLOCK_SIZE);
    int initial = 0;

    // traverse bitmap in search for empty block
    for (k = begin; k < POINTERS_PER_INODE; k++) {
    	for (l = block.super.ninodeblocks + 1; l < bitmapSize; l++) {
            // check if free
    		if (fbbitmap[l] == 0) {
    			inode.direct[k] = l;
                // mark as taken
    			fbbitmap[l] = 1;

    			break;
    		}
    	}

        // using offset would result in errors
        int actualOffset = offset % 4096;
        // if not intial write
    	if (initial == 0) {
    		disk_read(inode.direct[k], block.data);
            // work through bytes until length is reached 
    		for (m = 0; m + actualOffset < DISK_BLOCK_SIZE; m++) {
    			block.data[m + actualOffset] = data[byte];
    			byte++;
                // if length is reached
    			if (byte == length) {
    				disk_write(inode.direct[k], block.data);
    				inode.size = byte + offset;
                    // save inode
    				inode_save(inumber, &inode);
                    // return total bytes written
    				return byte;
    			}
    		}

            // update block
    		disk_write(inode.direct[k], block.data);
            initial = 1;

    	} else { // initial is true
    		disk_read(inode.direct[k], block.data);
            // work through bytes until length is reached
    		for (n = 0; n < DISK_BLOCK_SIZE; n++) {
    			block.data[n] = data[byte];
    			byte++;
    			if (byte == length) {
                    // update block
    				disk_write(inode.direct[k], block.data);
    				inode.size = byte + offset;
                    // update inode 
    				inode_save(inumber, &inode);
                    // return total bytes written
    				return byte;
    			}
    		}
            // update block
    		disk_write(inode.direct[k], block.data);
    	}
    }

	// traverse bitmap in search for empty block
	int x, y, z, a;
	for (x = block.super.ninodeblocks + 1; x < bitmapSize; x++) {
        // if block is free
		if (fbbitmap[x] == 0) {
			inode.indirect = x;
            // mark as taken
			fbbitmap[x] = 1;

			break;
		}
	}

    // Indirect block
	if (inode.indirect) {
        // read indirect block
		disk_read(inode.indirect, iBlock.data);
        // traverse bitmap in search for empty block
        int iBegin = begin - 5;
		for (y = iBegin; y < POINTERS_PER_BLOCK; y++) {
			// traverse bitmap in search for empty block
			for (z = block.super.ninodeblocks + 1; z < bitmapSize; z++) {
                // if block is free
				if (fbbitmap[z] == 0) {
					iBlock.pointers[y] = z;
                    // mark as taken
					fbbitmap[z] = 1;

					break;
				}
			}

            // read and traverse "pointers"
			disk_read(iBlock.pointers[y], block.data);
            // work through these bytes until length is reached 
			for(a = 0; a < DISK_BLOCK_SIZE; a++) {
                // update block.data
				block.data[a] = data[byte];
				byte++;
				if (byte == length) {
					disk_write(iBlock.pointers[y], block.data);
					inode.size = byte + offset;
                    // save inode w/ inumber
					inode_save(inumber, &inode);
                    // return total bytes written
					return byte;
				}
			}
            // update block
			disk_write(iBlock.pointers[y], block.data);
		}

        // update iblock
		disk_write(inode.indirect, iBlock.data);
	}

	// update the inode
	inode.size = byte + offset;
	inode_save(inumber, &inode);

    // Return total bytes written
	return byte;
}
