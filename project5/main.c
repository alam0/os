/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Global Variables:
int physicalFrameNumber = 0;
int pageFaults = 0;
int reads = 0;
int writes = 0;
int npages;
int nframes;
const char *algorithm;
char *virtmem;
char *physmem;
struct disk *disk1;
struct page_table *pt;
int eviction = 0;
int frame;
int bits;

// QUEUE Structure in C
struct node {
    int info;
    struct node *ptr;
} *head, *tail, *temp, *front;

int count = 0;

// Create empty queue
void create() {
    head = NULL;
    tail = NULL;
}

int headElement() {
    if ((head != NULL) && (tail != NULL)) {
        return (head->info);
    } else {
        return 0;
    }
}

int tailElement() {
    if ((head != NULL) && (tail != NULL)) {
        return (tail->info);
    } else {
        return 0;
    }
}

// Enqueue
void enq(int data) {
    if (tail == NULL){
        tail = (struct node *)malloc(1*sizeof(struct node));
        tail->ptr = NULL;
        tail->info = data;
        head = tail;
    } else {
        temp = (struct node*)malloc(1*sizeof(struct node));
        tail->ptr = temp;
        temp->info = data;
        temp->ptr = NULL;
        tail = temp;
    }
    count++;
}

// Dequeue the queue
void deq() {
    front = head;
    if (front == NULL) {
        printf("error dequeing from empty queue");
    } else if (front->ptr != NULL) {
        front = front->ptr;
        printf("Dequed: %d", head->info);
        free(head);
        head = front;
    }
    count--;
}

// Display queue
void display() {
    front = head;
    if ((front == NULL) && (tail == NULL)) {
        printf("Queue empty");
        return;
    }
    while (front != tail) {
        printf("%d", front->info);
        front = front->ptr;
    }
    if (front == tail)
        printf("%d", front->info);
}
// check if queue empty
int empty() {
    if ((head == NULL) && (tail == NULL)) {
        return 1;
    } else {
        return 0;
    }
}


void page_fault_handler( struct page_table *pt, int page ) {
    pageFaults++;
// ALGORITHMS
    // create a frame table that keeps track of the state of each frame.
    int frameTable[nframes]; // 0 for empty, 1 for not    
    int frameTable1[nframes];
    int emptyFrames[nframes]; // 1 for empty, 0 for not
    int numEmpty = 0;
    int evict_page;
    int evict_frame;
    int random_frame;
    // RAND
    if(!strcmp(algorithm,"rand")) {
        printf("rand");
        
        // get number of empty frames
        int i;
        for (i = 0; i < nframes; i++) {
            if (frameTable[i] == 0) {
                // mark as empty
                emptyFrames[i] = i;
                // increment number of empty frames for random number
                numEmpty = numEmpty + 1;
            }
        }
        // if no empty frames (numEmpty = 0), choose one at random to evict
        if (numEmpty == 0) {
            int randIndex = rand() % nframes;
            evict_page = frameTable1[randIndex];
            page_table_print(pt);
            evict_frame = pt->page_mapping[evict_page];
            eviction = 1;
            
        } else { // there are empty frames
            // get array of size numEmpty of just the empty frame numbers
            int filteredFrames[numEmpty];
            int frameNumber = 0;
            int j;
            for (j = 0; j < nframes; j++) {
                if (emptyFrames[j] != 0) {
                    filteredFrames[frameNumber] = emptyFrames[j];
                    frameNumber = frameNumber + 1;
                }
            }
            
            int randIndex = rand() % numEmpty;
            random_frame = filteredFrames[randIndex];
        }
        // Page and Frame found, now work with PT
        
        // Tell if page is written in disk or in mem
        page_table_get_entry(pt, page, &frame, &bits);
    
        // attempt to read a page without PROT_READ
        if (bits == 0) {
            // if frame is evicted
            if (eviction == 1) {
                // write evict_page back to the disk 
                disk_write( disk1, evict_page, &physmem[evict_frame*PAGE_SIZE]);
                writes++;
                
                // page is new page to read to frame where evict_page was
                disk_read(disk1, page, &physmem[evict_frame*PAGE_SIZE]);
                reads++;
                
                // page new page into physical memory.
                page_table_set_entry(pt, page, evict_frame, PROT_READ);
                
                // Set arrays to keep track of page
                emptyFrames[evict_frame] = 0;
                frameTable1[evict_frame] = page;
                
                // when you swap a page out of memory, make sure to set the bits to 0 to mark that it isn't in memory anymore
                page_table_set_entry(pt, evict_page, evict_frame, 0);
                
            } else { // There is a free frame present. 
                // adjust page table to map page # to frame #, with PROT_READ
                page_table_set_entry(pt, page, random_frame, PROT_READ);
                
                emptyFrames[random_frame] = 0;
                frameTable1[random_frame] = page;
                
                // load page # from disk into frame #
                disk_read(disk1, page, &physmem[random_frame*PAGE_SIZE]);
                reads++;
            }
        // attempt to write to page with only PROT_READ set, add PROT_WRITE
        } else if (bits == PROT_READ) {
            // grab frame of page
            int curr_frame = pt->page_mapping[page];
            // adjust page table to map page # to frame #, with PROT_READ|PROT_WRITE
            page_table_set_entry(pt, page, curr_frame, PROT_READ|PROT_WRITE);
        } 

    // FIFO
	} else if(!strcmp(algorithm,"fifo")) {
	/* for FIFO, everytime you add a page to physical memory, add the page to your queue. Everytime you remove a page from physical memory, remove that page from the front of the queue
*/      
        // when we load page 2 from disk to physmem
        //disk_read(disk, 2, &physmem[3.frame_size]);
        
        // Create Queue
        create();
        
        // Start off by just filling up the possible frames
        if (physicalFrameNumber < nframes - 1) {
            printf("loop");
            // Tell if page is written in disk or in mem
            page_table_get_entry(pt, page, &frame, &bits);
            // attempt to read a page without PROT_READ
            if (bits == 0) {
                page_table_set_entry(pt, page, physicalFrameNumber, PROT_READ);
            
                disk_read(disk1, page, &physmem[physicalFrameNumber*PAGE_SIZE]);
                enq(page);
                reads++;
                
                physicalFrameNumber++;
            // attempt to write to page with only PROT_READ set, add PROT_WRITE
            } else if (bits == PROT_READ) {
                // grab frame of page
                int curr_frame = pt->page_mapping[page];
                // adjust page table to map page # to frame #, with PROT_READ|PROT_WRITE
                page_table_set_entry(pt, page, curr_frame, PROT_READ|PROT_WRITE);
            } 
            
        // Frames filled  
        } else {
            // actual FIFO
            // Tell if page is written in disk or in mem
            page_table_get_entry(pt, page, &frame, &bits);
            // attempt to read a page without PROT_READ, page not in memory
            if (bits == 0) {
                int first_page = tailElement();
                int first_frame = pt->page_mapping[first_page];
                // write evict_page back to the disk 
                disk_write( disk1, first_page, &physmem[first_frame*PAGE_SIZE]);
                writes++;
                // page is new page to read to frame where evict_page was
                disk_read(disk1, page, &physmem[first_frame*PAGE_SIZE]);
                reads++;
                
                // page new page into physical memory.
                page_table_set_entry(pt, page, first_frame, PROT_READ);
                // when you swap a page out of memory, make sure to set the bits to 0 to mark that it isn't in memory anymore
                page_table_set_entry(pt, first_page, first_frame, 0);
                
            // attempt to write to page with only PROT_READ set, add PROT_WRITE
            } else if (bits == PROT_READ) {
                // grab frame of page
                int curr_frame = pt->page_mapping[page];
                // adjust page table to map page # to frame #, with PROT_READ|PROT_WRITE
                page_table_set_entry(pt, page, curr_frame, PROT_READ|PROT_WRITE);
            } 
        }
        
	} else if(!strcmp(algorithm,"custom")) {
		printf("custom");

	} else {
		fprintf(stderr,"unknown algorithm");
		exit(1);
	}
// END ALGORITHMS

}


int main( int argc, char *argv[] )
{   
    // Error checking
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		return 1;
	}
	
    // Set up global variables
	npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	
	if (nframes > npages) {
	    nframes = npages;
	}
	
	algorithm = argv[3];
	const char *program = argv[4];

	disk1 = disk_open("myvirtualdisk",npages);
	if(!disk1) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

    // Page Table
	pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}
	
    
	virtmem = page_table_get_virtmem(pt);
	physmem = page_table_get_physmem(pt);
    
    // SORT, SCAN, FOCUS
	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);
	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);
	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);
	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);
		return 1;
	}
	
	// Doesn't reach here
    printf("hi");
    printf("Page Faults: %d\n", pageFaults);
    printf("Reads: %d\n", reads);
    printf("Writes: %d\n", writes);
    
    deq();
	page_table_delete(pt);
	disk_close(disk1);
    
    
	return 0;
}
