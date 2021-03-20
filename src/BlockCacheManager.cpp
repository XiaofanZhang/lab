#include "BlockCacheManager.h"
#include "URLEntry.h"
#include "MurmurHash3.h"

#define BLOCK_SIZE 4096
#define FILE_NUMBER 100
#define COUNT_SIZE sizeof(uint64_t)

// Writes number into char* buffer
static void number2char(uint64_t number, char * buf) {
    for (size_t i = 0; i < COUNT_SIZE; ++i) {
        buf[i] = number & 0xFF;
        number >>= 8;
    }
}

// Read number back char* buffer
static uint64_t char2number(const char * buf) {
    uint64_t number = 0;
    size_t i = COUNT_SIZE;
    while (i--) number = (number<<8) | buf[i];
    return number;
}

BlockCacheManager::BlockCacheManager() {
    fds = vector<FILE*>(FILE_NUMBER, NULL);
    offsets = vector<uint64_t>(FILE_NUMBER, 0);

    read_buffers = vector<char*>(FILE_NUMBER, NULL);
    write_buffers = vector<char*>(FILE_NUMBER, NULL);

    for(int i = 0; i < FILE_NUMBER; i++) {
        string name = "partition";
        name += to_string(i);
        fds[i] = fopen(name.data() , "w+");
        read_buffers[i] = new char[BLOCK_SIZE];
        write_buffers[i] = new char[BLOCK_SIZE];
    }
}

BlockCacheManager::~BlockCacheManager() {
    for(int i = 0; i < FILE_NUMBER; i++) {
        delete read_buffers[i];
        delete write_buffers[i];
        fclose(fds[i]);
    }
}

URLEntry* BlockCacheManager::findURL(char *url, size_t len) {
    uint64_t hash;
    MurmurHash3_x86_32(url, len, 0, &hash);
    int pos = hash % FILE_NUMBER;
    fseek(fds[pos], 0, SEEK_SET);

    bool find = false;
    int num = 0;
    int prev = 0;

    // before reading from disk, first check our write buffer
    for(int i = 0; i < offsets[pos]; i++) {
        if(write_buffers[pos][i] == '\0' ) {
            // found it, delete from write buffer and put it back to mem
            if( i - prev  == len + 1 && strncmp(url, write_buffers[pos] + prev, len) == 0) {
                struct URLEntry * entry = new URLEntry(url, len);
                entry->count = char2number(write_buffers[pos] + i + 1);
                // printf("remove from write buffer url: %s count: %llu\n", entry->data, entry->count);
                if(i + COUNT_SIZE + 1 < offsets[pos]){
                    memcpy(write_buffers[pos] + prev, write_buffers[pos] + i + COUNT_SIZE + 1, offsets[pos] - i - 1 - COUNT_SIZE);
                }
                offsets[pos] = offsets[pos] - (len + 1 + COUNT_SIZE);
                return entry;
            }
            prev = i + 1 + COUNT_SIZE;
        }
    }

    while(!find) {
        size_t size = fread(read_buffers[pos], 1, BLOCK_SIZE, fds[pos]);
        if(size < 1) {
            return NULL;
        }
        
        prev = 0;
        for(int i = 0; i < BLOCK_SIZE;) {
            size_t length = strlen(read_buffers[pos] + i);
            // printf("%s\n", read_buffers[pos] + i);
            // found, construct entry from disk
            if( length == len  && strncmp(read_buffers[pos] +i ,url, len) == 0) {
                struct URLEntry * entry = new URLEntry(url, len);
                entry->count = char2number(read_buffers[pos] + i + length + 1);
                entry->count_offset = num * BLOCK_SIZE + i + length + 1;
                entry->persisted = true;
                // printf("read url: %s count: %llu\n", entry->data, entry->count);
                return entry;
            }
            if(length == 0)
                break;
            i = i + length + 1 + COUNT_SIZE;
        }
        num ++;
    }
    
    // false positive
    return NULL;
}

// take the evicted URLs and persist to disk if needed
void BlockCacheManager::persist(struct URLEntry *entry) {
    uint64_t hash;
    MurmurHash3_x86_32(entry->data, entry->len - 1, 0, &hash);
    int pos = hash % FILE_NUMBER;
    // printf("persisiting url: %s to pos: %d count: %llu\n", entry->data, pos, entry->count);
    if(!entry->persisted) {
        // persist both url and count for the first time
        if(offsets[pos] + entry->len + COUNT_SIZE >= BLOCK_SIZE) {
            // persist current block and restart
            // fill in the remaining of this block with 0
            memset(write_buffers[pos] + offsets[pos], 0, BLOCK_SIZE - offsets[pos]);
            fseek(fds[pos], 0, SEEK_END);
            fwrite(write_buffers[pos], 1, BLOCK_SIZE, fds[pos]);
            offsets[pos] = 0;
        }
        // write to buffer
        strcpy(write_buffers[pos] + offsets[pos], entry->data);

        char buf[COUNT_SIZE];
        number2char(entry->count, buf);
        memcpy(write_buffers[pos] + offsets[pos] + entry->len, buf, COUNT_SIZE);

        offsets[pos] += entry->len + COUNT_SIZE;
        delete entry; 
    } else {
        // url is already on disk, only need to update count
        char buf[COUNT_SIZE];
        number2char(entry->count, buf);
        fseek(fds[pos], entry->count_offset, SEEK_SET);
        fwrite(buf, 1, COUNT_SIZE, fds[pos]);
        delete entry;
    }
}