# Problem:
There is a 100 GB file of URLs on the disk. Please write a small project to calculate the top 100 most common URLs and their frequency respectively in the file within 1 GB of memory.

# Requirement:
The program should not consume larger than 1 GB of memory, the faster the better

# Expected time:
1 week, started from Mar.16 2021

# Thoughts:
1. The most common solution for this problem is map-reduce with max-heap from bigdata, break the large file into partitions that can be fitted into memroy, count and combining them. This require first read the data once, partition and write it back, then need to read the whole data again for counting and write the count back. Finally read the count again and merge them. 
2. Initially I was thinking we should also count when we are reading data into different partitions, and that raises the question on how we should put URLs from memory to file when we hit the memory limits. This reminds me of the memory management unit of OS where the page table keep tracks of which page is swapped to file and swapped it back when it is accessed. There is many algorithms that can be used here, FIFO, LRU and LFU etc.
    - If we take another step back and put the memory limit aside, think each url as the memory address we want to access and choose least frequency used as the evict algorithm through the whole time (DO NOT reset frequency even it is evicted), then after reading the file the most common URLs will already be in the memory. If we could have a perfect hash function for URLs without collisions, this could be easily solved by using the hash as memory key. Because of the potential collisions, we have to use the URLs as the key directly so that we can compare them.

    - Because of the memory limit, we need to use filesystem for URLs evicted from the memory and it will suffer if URLs are evicted and loaded to memory back and forth. Not to mention that we have to scan the disk when we are trying to find one URL if it is not in memory.

    - To avoid unnecessary read of disk, we use bloom filter to check whether we have seen this URL before. If so, we will first check memory then disk. When looking up URL on disk, we simply scan the file from the start right now but we should use something else like B-tree to improve that as well. I failed to figure that out with a proper approach, so I simply use 100 file by hash of url to minimize the search.

3. Even though the LFU did the job, it suffers if URLs does not show up at all in the first half and start to building up in the second half, because it does not consider recency. So I dig around and found something interesting, adaptive replacmenet cache (a.k.a ARC) [^1] which combines frequency and recency, it saves us from the unnecessnary disk IO due to burst appearances of some recent URLs. In our case I don't think we need to keep the ghost list because our key takes too much space, so we simply take the idea and combine the LRU with LFU to achieve a simplified version of ARC.

So we got ourselves a plan and the next step is to open the refrigerator and put the elephant in. ~~Piece of cake! (dog~~

# Improvements:
1. I did not malloc a bunch of memory and manage them manually for URLs, this leads to memory fragmentation in the case of many and small URLs.
2. Depending on the URLs pattern in the file, this code works well if you have lots of one-time URLs and some hot URLs that can be fitted into memory, but it will suffer if the data is distributed evenly across the URLs in which case involves too much unnecessnary disk IO. I can easily sabotage this approach by arrange the data repeatedly in a pattern that it will evict the final common URLs back and forth.
3. For the worst case, we could replace this with the big data approach because evenly distributed data partitions sounds perfect for it. By running two solutions at the same time and waiting for the first to finish, our solution might be 2x or more slower in common cases, but the worst case performance will be improved significantly.

# Test:
I tested this on my mac with 100MB file, generate 100MB file with random string and cut it to multiple lines with sed. Process that take one minute or two. If to be tesed on larger file, should configure FILE_NUMBER, BLOOM_FILTER_SIZE and list size accordingly.
```
dd if=/dev/urandom bs=100000000 count=1 | base64 > /tmp/file
```

[^1]: https://en.wikipedia.org/wiki/Adaptive_replacement_cache