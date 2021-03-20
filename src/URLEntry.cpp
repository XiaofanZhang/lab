#include "URLEntry.h"

// WARNING: memory fragmentation
URLEntry::URLEntry(char *url, size_t length)  
    : len(length + 1), count(1), persisted(false) {
    data = (char *)malloc(length+1);
    memcpy(data, url, length);

    // append the end character for later usage
    data[length] = '\0';
}

URLEntry::~URLEntry() {
    delete data;
}