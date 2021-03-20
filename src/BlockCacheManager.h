#ifndef _BLOCKCACHEMANAGER_H_
#define _BLOCKCACHEMANAGER_H_

#include <vector>
#include <list>
#include <string_view>
#include <string>

using namespace std;

class BlockCacheManager {
    public:
        BlockCacheManager();
        ~BlockCacheManager();
        vector<FILE*> fds;
        // buffer for reading URL from disk
        vector<char*> read_buffers;
        vector<char*> write_buffers;
        vector<uint64_t> offsets;

        // take the evicted url and its count, persist them to disk if full
        void persist(struct URLEntry*);
        URLEntry* findURL(char * url, size_t len);
};

#endif // _BLOCKCACHEMANAGER_H_
