#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BloomFilter.h"
#include "EasyARC.h"
#include "URLEntry.h"

/*
* http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html
* Depending on the number of unique URLs(here we assume it is 10,000,000 at most)
* the false positive probability of bloom filter for 8 hash functions is 0.0000889
*/

#define BLOOM_FILTER_SIZE 8 * 25 * 1024 * 1024
#define BUF_SIZE 4 * 1024
#define NUM_HASHES 8

char buf[BUF_SIZE];

uint64_t lineno = 0;

// Get url of each line from buffer 
int process(EasyARC *arc, struct BloomFilter *bf, size_t size, bool finished) {
    int pos = 0;
    for(size_t i = 0; i < size; i++) {
        if(buf[i] == '\n' || (i + 1 == size && finished)) {
            lineno++;
            // printf("%llu %.*s  \n", lineno, int(i - pos), buf + pos);
            if(!bf->contains(buf + pos, i - pos)) {
                // never met this url before
                bf->add(buf + pos, i - pos);
                struct URLEntry *entry = new URLEntry(buf + pos, i - pos);
                arc->insert(entry);
                
            } else {
                arc->access( buf + pos, i - pos);
            }
            arc->evict();
            pos = i + 1;
        }
    }
    return pos;
}

int main(int argc, char *argv[]) {
    
    if(argc < 3) {
        printf("You need to provide the input file name and K number!");
        printf("Usage: ./lab inputFile K");
	    exit(EXIT_FAILURE);
    }

    struct BloomFilter *bf = new BloomFilter(BLOOM_FILTER_SIZE, NUM_HASHES);
    EasyARC *arc = new EasyARC();
    size_t size;
    int leftover= 0;
    FILE *fp = NULL;
    bool finished = false;
    if (!(fp = fopen(argv[1], "r"))) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    // read file in chunk
    do {
        size = fread(buf + leftover, 1, sizeof(buf) - leftover, fp);
        if(size < 1) {
            finished = true;
            size = 0;
        }

        int pos = process(arc, bf, size + leftover, finished);
        leftover = size + leftover - pos;
        if ( pos != 0 && leftover != 0)
	        memmove(buf, buf+pos, leftover);
    } while(!finished);

    printf("===============================================\n");
    printf("total file line: %llu\n", lineno);
    printf("total unique url: %llu\n", arc->unique_count);
    printf("frequent cache hit: %llu\n", arc->freq_cache_hits);
    printf("recent cache hit: %llu\n", arc->recent_cache_hits);
    printf("disk read times: %llu\n", arc->disk_scan);
    printf("bloom filter false positive times: %llu\n", arc->false_positive);
    printf("===============================================\n");
    arc->outputTopK(stoi(argv[2]));
    fclose(fp);
}

