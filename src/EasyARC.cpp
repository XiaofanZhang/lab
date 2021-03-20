#include "EasyARC.h"
#include "URLEntry.h"

#define MEMORY_THRESHOLD_CEILING 600 * 1024 * 1024
#define MEMORY_THRESHOLD_FLOOR 500 * 1024 * 1024

// control list size for testing purposes
#define MAX_LIST_SIZE 2000000
#define MIN_FREQ_SIZE 300

EasyARC::EasyARC() : url_size(0), false_positive(0),
    unique_count(0), freq_cache_hits(0), recent_cache_hits(0), disk_scan(0) {}

EasyARC::~EasyARC() {
    for (auto i: freq)
        delete i;
    for (auto i: recent)
        delete i; 
}

inline uint64_t EasyARC::mem_size() {
    return url_size + (freq.size() + recent.size()) * sizeof(struct URLEntry);
}

inline bool EasyARC::checkMem() {
    return mem_size() > MEMORY_THRESHOLD_CEILING;
}

URLEntry * EasyARC::load(char *data, size_t len) {
    return manager.findURL(data, len);
}

// the input data does not have end character which we manually added
URLEntry * EasyARC::find(char *data, size_t len) {
    for(auto it = freq.begin(); it != freq.end(); it++) {
        if(len + 1 == (*it)->len && strncmp(data, (*it)->data, len) == 0) {
            URLEntry * temp = *it;
            freq.erase(it);
            freq_cache_hits++;
            return temp;
        }
    }
    for(auto it = recent.begin(); it != recent.end(); it++) {
        if(len + 1 == (*it)->len && strncmp(data, (*it)->data, len) == 0) {
            URLEntry * temp = *it;
            recent.erase(it);
            recent_cache_hits++;
            return temp;
        }
    }
    return NULL;
}

// url could be in the list or disk
void EasyARC::access(char * data, size_t len) {
    // iterate from memory first
    struct URLEntry *entry = find(data, len);
    if(entry == NULL) {
        // damn, need to scan disk
        disk_scan++;
        entry = load(data, len);
        if(entry == NULL) {
            // not found, bloom fiter gives false positive
            false_positive++;
            // printf("false positive for %.*s \n", len, data);
            struct URLEntry *entry = new URLEntry(data, len);
            insert(entry);
            return;
        }
    }

    entry->count = entry->count + 1;

    // check if it can make it to freq 
    if(freq.size() < MIN_FREQ_SIZE || entry->count >= freq.back()->count) {
        // insert this to proper location
        for(auto it = freq.begin(); it != freq.end(); it++) {
            if( (*it)->count < entry->count) {
                freq.insert(it, entry);
                return;
            }
        }
        freq.push_back(entry);
    } else {
        recent.push_front(entry);
    }
}

// add the url for the first time
void EasyARC::insert(URLEntry * entry) {
    if(entry != NULL) {
        // simply add to front of recent list
        recent.push_front(entry);
    }
    unique_count ++;
}

void EasyARC::evict() {
    URLEntry * evicted = NULL;

    // evicting a lot of entries for better disk io
    while(mem_size() > MEMORY_THRESHOLD_FLOOR || freq.size() + recent.size() > MAX_LIST_SIZE) {
        // evict entries towards equal list size of freq and recent
        if(freq.size() < recent.size()) {
            //printf("evicting from recent\n");
            evicted = recent.back();
            recent.pop_back();
            manager.persist(evicted);
        } else {
            // entry from LFU will be removed
            //printf("evicting from freq\n");
            evicted = freq.back();
            freq.pop_back();
            manager.persist(evicted);
        }
    }
}

void EasyARC::outputTopK(int k) {
    printf("Output top K URLs:\n");
    while( k > 0 && !freq.empty()) {
        struct URLEntry * entry = freq.front();
        freq.pop_front();
        printf("%s %llu\n", entry->data, entry->count);
        k--;
    }

    if( k == 0)
        return;
    // URLs only appear once will never make it to freq
    while( k > 0 && !recent.empty()) {
        struct URLEntry * entry = recent.front();
        recent.pop_front();
        printf("%s %llu\n", entry->data, entry->count);
        k--;
    }
}