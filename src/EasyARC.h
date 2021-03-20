#ifndef _EASY_ARC_H_
#define _EASY_ARC_H_

#include <list>
#include "BlockCacheManager.h"
using namespace std;

#define MEMORY_THRESHOLD_CEILING 600 * 1024 * 1024
#define MEMORY_THRESHOLD_FLOOR 500 * 1024 * 1024

// can be made template
class EasyARC {
    public:
        EasyARC();
        ~EasyARC();

        // access the entry
        void access(char *, size_t);
        // insert new entry
        void insert(URLEntry *);
        // load url from disk
        URLEntry* load(char *data, size_t len);
        // find url from memory
        URLEntry* find(char *data, size_t len);
        void outputTopK(int k);
        // evict entries towards equal size of freq and recent lists
        void evict();
        bool checkMem();
        uint64_t mem_size();
        // LFU list
        list<URLEntry *> freq;
        // LRU list
        list<URLEntry *> recent;
        uint64_t url_size;
        BlockCacheManager manager;

        // some statistics
        uint64_t false_positive;
        uint64_t unique_count;
        uint64_t freq_cache_hits;
        uint64_t recent_cache_hits;
        uint64_t disk_scan;
        
};

#endif // _EASY_ARC_H_
