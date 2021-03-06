#include "BloomFilter.h"
#include "MurmurHash3.h"

BloomFilter::BloomFilter(uint64_t size, uint8_t numHashes) 
	: m_bits(size), m_numHashes(numHashes) {}

array<uint64_t, 2> compute(char *data, size_t len) {
	array<uint64_t, 2> hashValue;
	MurmurHash3_x64_128(data, len, 0, hashValue.data());

	return hashValue;
}

// Use double hashing to simulate k hash values
// hash(x, m) = (hash(x) + i * hash(x)) mod m
inline uint64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
	return (hashA + n * hashB) % filterSize;
}

void BloomFilter::add(char *data, size_t len) {
	array<uint64_t, 2> hashValues = compute(data, len);

  	for (int n = 0; n < m_numHashes; n++) {
    	m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())] = true;
  	}
}

bool BloomFilter::contains(char *data, size_t len) {
  	array<uint64_t, 2> hashValues = compute(data, len);

  	for (int n = 0; n < m_numHashes; n++) {
    	if (!m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())]) {
      		return false;
    	}
  	}

  	return true;
}
