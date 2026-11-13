/**
 * Name: Thomas Newman
 * Student Number: 221224662
 * Purpose: Simulating how a CPU reads from memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16

typedef struct {
    int page;
    int frame;
} TLBEntry;

void mmu(FILE *binaries, FILE *addresses, int frames, FILE *output) {
    
    int current_address;
    int page_table[256];
    int frame_counter = 0;
    int page_faults = 0;
    int tlb_index = 0;
    double total_addresses = 0;
    double tlb_hits = 0;

    int time = 0;
    int *last_used = malloc(frames * sizeof(int));
    int *frame_to_page = malloc(frames * sizeof(int));

    TLBEntry TLB[TLB_SIZE];
    // TLB initialization
    for (int i = 0; i < TLB_SIZE; i++) {
        TLB[i].page = -1;
    }

    //initialize physical memory
    signed char *physical_memory = malloc(frames * 256);

    if (physical_memory == NULL || last_used == NULL || frame_to_page == NULL) {
        printf("Error allocating memory.");
        return;
    }

    // initialize page table
    for (int i = 0; i < 256; i++) {
        page_table[i] = -1;
    }

    while (fscanf(addresses, "%d", &current_address) == 1) {
        int page_number = (current_address & 0xFF00) >> 8;
        int offset = current_address & 0xFF;

        int frame;
        int tlb_hit = 0;

        for (int i = 0; i < TLB_SIZE; i++) {
            if (TLB[i].page == page_number) {
                frame = TLB[i].frame;
                tlb_hit = 1;
                break;
            }
        }

        if (tlb_hit) {
            tlb_hits++;
        } else {
            if (page_table[page_number] == -1) { //page fault
                page_faults++;

                if (frame_counter < frames) { // free frame available
                    frame = frame_counter;
                    frame_counter++;
                } else { // memory full -> LRU
                    int lru_frame = 0;

                    for (int i = 1; i < frames; i++) {
                        if (last_used[i] < last_used[lru_frame]) {
                            lru_frame = i;
                        }
                    }

                    frame = lru_frame;
                    int old_page = frame_to_page[frame];
                    page_table[old_page] = -1;
                    for (int i = 0; i < TLB_SIZE; i++) {
                        if (TLB[i].page == old_page) {
                            TLB[i].page = -1;
                        }
                    }
                }

                page_table[page_number] = frame;
                fseek(binaries, page_number * 256, SEEK_SET);
                fread(&physical_memory[frame*256], 1, 256, binaries);
                frame_to_page[frame] = page_number;
            }
            frame = page_table[page_number];
            TLB[tlb_index].page = page_number;
            TLB[tlb_index].frame = frame;
            tlb_index = (tlb_index + 1) % TLB_SIZE; // increment tlb_index (basically a cache), if it hits the TLB_SIZE, it loops back around to zero.
        }

        last_used[frame] = time;
        time++;

        int physical_address = (frame << 8) + offset;
        int value = physical_memory[physical_address];

        total_addresses++;

        fprintf(output, "%d,%d,%d\n", current_address,physical_address,value);
    }
    double fault_rate = page_faults/total_addresses * 100;
    double tlb_hit_rate = tlb_hits/total_addresses * 100;
    fprintf(output, "Page Faults Rate,%.2f%%,\nTLB Hits Rate,%.2f%%,", fault_rate, tlb_hit_rate);

    free(physical_memory);
    free(last_used);
    free(frame_to_page);
}

int main (int argc, char *argv[]) {

    FILE *binaries, *addresses, *output;
    int frames;

    if (argc != 4) {
        printf("Incorrect amount of arguments, please try again.");
        return 1;
    }

    frames = atoi(argv[1]);

    char filename[32];
    snprintf(filename, 32, "output%d.csv", frames);

    binaries = fopen(argv[2], "rb");
    addresses = fopen(argv[3], "r");
    output = fopen(filename, "w");

    if (binaries == NULL || addresses == NULL || output == NULL) {
        printf("Could not open a file.");
        return 1;
    }

    mmu(binaries, addresses, frames, output);

    fclose(binaries);
    fclose(addresses);
    fclose(output);

    return 0;
}