#ifndef _URLENTRY_H_
#define _URLENTRY_H_
#include <array>

using namespace std;

// sorry, can't afford memory alignment now
#pragma pack(1)
struct URLEntry {
    URLEntry(char *url, size_t length);
    ~URLEntry();
    char* data;
    bool persisted;
    size_t len;
    uint64_t count;
    uint64_t count_offset;
};
#pragma pack()

#endif // _URLENTRY_H_