#ifndef _BLOOMFILTER_H_
#define _BLOOMFILTER_H_

#include <vector>
#include <array>
using namespace std;

struct BloomFilter {
    BloomFilter(uint64_t size, uint8_t numHashes);
    void add(char *data, size_t len);
    bool contains(char *data, size_t len);
private:
    uint8_t m_numHashes;
    vector<bool> m_bits;
};

#endif // _BLOOMFILTER_H_
