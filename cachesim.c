#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    int valid; //Determines whether a cache block has data that is valid or not
    unsigned long tag; //identifies the unique memory block stored in a specific cache line
    int age; //used for First In First Out and Least Recently Used replacement policies.
} CacheLine;

typedef struct {
    CacheLine* cachelines; //pointer to an array of cachelines
} CacheSet;

typedef struct {
    CacheSet* sets; //pointer to array of cachesets.
    int setCount; //number of sets
    int assoc; //Associativity (number of lines per set)
    int blockSize; //size of block in bytes
} Cache;

//Struct used to track statistics which are memory reads, memory writes, cache hits and misses.
typedef struct {
    int memoryReads;
    int memoryWrites;
    int cacheHits;
    int cacheMisses;
} Stats;

//this method extracts the block ID from the memory address in the form of an unsigned long.
unsigned long getBlockID(unsigned long addr, int blockBits) {
    return addr >> blockBits;
}

//Extracts the set index from the block ID 
unsigned long getSetIndex(unsigned long blockID, int setBits) {
    if (setBits == 0) return 0;
    return blockID & ((1 << setBits) - 1);
}

//Extracts the tag from the block ID
unsigned long getTag(unsigned long blockID, int setBits) {
    return blockID >> setBits;
}


//sets up and initializes the cache structure, using memory allocation
Cache* initializeCache(int sets, int assoc) {
    Cache* cacheStructure = malloc(sizeof(Cache));
    cacheStructure->setCount = sets;
    cacheStructure->assoc = assoc;
    cacheStructure->sets = malloc(sizeof(CacheSet) * sets);
    for (int i = 0; i < sets; i++) {
        cacheStructure->sets[i].cachelines = malloc(sizeof(CacheLine) * assoc);
        for (int j = 0; j < assoc; j++) {
            cacheStructure->sets[i].cachelines[j].valid = 0;
            cacheStructure->sets[i].cachelines[j].tag = 0;
            cacheStructure->sets[i].cachelines[j].age = 0;
        }
    }
    return cacheStructure;
}

//frees up dynamically allocated memory from the cache 
void freeCache(Cache* cache) {
    for (int i = 0; i < cache->setCount; i++) {
        free(cache->sets[i].cachelines);
    }
    free(cache->sets);
    free(cache);
}

//checks if an integer is a power of two or not
int powerOfTwo(int n) {
    return n && !(n & (n - 1));
}

// Forward declaration for the prefetch function

void doPrefetch(Cache* cache, unsigned long addr, Stats* stats, int lru);

//handles the accessing of a single cache
void accessCache(Cache* cache, unsigned long addr, int isWrite, Stats* stats, int prefetch, int lru) {
    int blockBits = (int)log2(cache->blockSize); //num of bits for block offset
    int setBits = (int)log2(cache->setCount); //num of bits for the set index

    //breaks down the address
    unsigned long block = getBlockID(addr, blockBits);
    unsigned long setIndex = getSetIndex(block, setBits);
    unsigned long tag = getTag(block, setBits);

    CacheSet* set = &cache->sets[setIndex];

    // Searches for a cache hit
    for (int i = 0; i < cache->assoc; i++) {
        if (set->cachelines[i].valid && set->cachelines[i].tag == tag) {
            stats->cacheHits++;
            if (isWrite)
                stats->memoryWrites++;
            if (lru) { //update the age factor for the least recently used policy
                int prevAge = set->cachelines[i].age;
                set->cachelines[i].age = 0;
                for (int j = 0; j < cache->assoc; j++) {
                    if (j != i && set->cachelines[j].valid && set->cachelines[j].age < prevAge)
                        set->cachelines[j].age++;
                }
            }
            return; //signifies a cache hit 
        }
    }
    //Miss handling
    stats->cacheMisses++;
    stats->memoryReads++;
    if (isWrite)
        stats->memoryWrites++;

    // Try to load into an invalid line first
    for (int i = 0; i < cache->assoc; i++) {
        if (!set->cachelines[i].valid) { //checks whether an invalid line is within the cache set.
            set->cachelines[i].valid = 1;
            set->cachelines[i].tag = tag;
            set->cachelines[i].age = 0;
            for (int j = 0; j < cache->assoc; j++) {
                if (j != i && set->cachelines[j].valid)
                    set->cachelines[j].age++;
            }
            if (prefetch)
                doPrefetch(cache, addr + cache->blockSize, stats, lru);
            return;
        }
    }
    //if no free line exists, we replace the oldest line (using either FIFO or lru)
    int replaceIndex = 0;
    int maxAge = set->cachelines[0].age;
    for (int i = 1; i < cache->assoc; i++) {
        if (set->cachelines[i].age > maxAge) {
            maxAge = set->cachelines[i].age;
            replaceIndex = i;
        }
    }

    set->cachelines[replaceIndex].tag = tag;
    set->cachelines[replaceIndex].age = 0; 
    for (int i = 0; i < cache->assoc; i++) {
        if (i != replaceIndex && set->cachelines[i].valid)
            set->cachelines[i].age++;
    }

    if (prefetch)
        doPrefetch(cache, addr + cache->blockSize, stats, lru);
}

// Prefetching function: load the next block

void doPrefetch(Cache* cache, unsigned long addr, Stats* stats, int lru) {
    int blockBits = (int)log2(cache->blockSize);
    int setBits = (int)log2(cache->setCount);

    unsigned long block = getBlockID(addr, blockBits);
    unsigned long setIndex = getSetIndex(block, setBits);
    unsigned long tag = getTag(block, setBits);

    CacheSet* set = &cache->sets[setIndex];

    for (int i = 0; i < cache->assoc; i++) {
        if (set->cachelines[i].valid && set->cachelines[i].tag == tag)
            return;
    }

    stats->memoryReads++;

    //Looks for a free line
    for (int i = 0; i < cache->assoc; i++) {
        if (!set->cachelines[i].valid) {
            set->cachelines[i].valid = 1;
            set->cachelines[i].tag = tag;
            set->cachelines[i].age = 0;
            for (int j = 0; j < cache->assoc; j++) {
                if (j != i && set->cachelines[j].valid)
                    set->cachelines[j].age++;
            }
            return;
        }
    }
    //if no free line exists, replace the oldest line (FIFO or lru)
    int replaceIndex = 0;
    int maxAge = set->cachelines[0].age;
    for (int i = 1; i < cache->assoc; i++) {
        if (set->cachelines[i].age > maxAge) { //checks if the specific age of the cacheline exceeds the oldest age
            maxAge = set->cachelines[i].age; //if so, it sets the max age to the cacheline age
            replaceIndex = i; //gets the index for that cacheline age 
        }
    }

    set->cachelines[replaceIndex].tag = tag;
    set->cachelines[replaceIndex].age = 0;
    for (int i = 0; i < cache->assoc; i++) {
        if (i != replaceIndex && set->cachelines[i].valid)
            set->cachelines[i].age++;
    }
}

//main function
int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("error\n");
        return 1;
    }

    int cacheSize = atoi(argv[1]); //cache byte size as the second argument passed (cache size is an integer)
    char* associativity = argv[2]; //associativity as the third argument passed (associativity is a string)
    char* repPolicy = argv[3]; //replacement policy as fourth argument passed (lru or FIFO)
    int blockSize = atoi(argv[4]); //block size as the fifth argument passed (block size is also an integer)

    if (!powerOfTwo(cacheSize) || !powerOfTwo(blockSize)) { //checks if cache or block size is or is not a power of 2.
        printf("error\n");
        return 1;
    }

    int assoc = 0;
    int sets = 0;

    if (strcmp(associativity, "direct") == 0) {
        assoc = 1;
        sets = cacheSize / blockSize;
    } else if (strncmp(associativity, "assoc:", 6) == 0) {
        assoc = atoi(associativity + 6); 
        if (!powerOfTwo(assoc)) {
            printf("error\n");
            return 1;
        }
        sets = cacheSize / (blockSize * assoc);
    } else if (strcmp(associativity, "assoc") == 0) {
        sets = 1;
        assoc = cacheSize / blockSize; //calculating associativity
    } else {
        printf("error\n");
        return 1;
    }
    //replacement policy
    int lru = 0;
    if (strcmp(repPolicy, "lru") == 0) {
        lru = 1;
    } else if (strcmp(repPolicy, "fifo") != 0) {
        printf("error\n");
        return 1;
    }
    //opens the file 
    FILE* fp = fopen(argv[5], "r");
    if (!fp) {
        printf("error\n");
        return 1;
    }

    Cache* cache0 = initializeCache(sets, assoc);
    Cache* cache1 = initializeCache(sets, assoc);
    cache0->blockSize = blockSize;
    cache1->blockSize = blockSize;

    Stats stats0 = {0}; //statistics for when prefetch is 0
    Stats stats1 = {0}; //statistics for when prefetch is 1

    char instr; //declares the type of memory operation (R or W)
    unsigned long addr; //memory address
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (buffer[0] == '#')
            break;
        sscanf(buffer, "%*x: %c %lx", &instr, &addr);
        if (instr == 'R') { //if the memory is being read
            accessCache(cache0, addr, 0, &stats0, 0, lru);
            accessCache(cache1, addr, 0, &stats1, 1, lru);
        } else if (instr == 'W') { //if the memory is being written
            accessCache(cache0, addr, 1, &stats0, 0, lru);
            accessCache(cache1, addr, 1, &stats1, 1, lru);
        }
    }

    fclose(fp); //closes the given file.

    //prints out the prefetch (0 or 1), and the specific memory reads, writes, cache hits and misses from the file based on whether there is prefetching or not
    printf("Prefetch 0\n"); //without prefetch
    printf("Memory reads: %d\n", stats0.memoryReads);
    printf("Memory writes: %d\n", stats0.memoryWrites);
    printf("Cache hits: %d\n", stats0.cacheHits);
    printf("Cache misses: %d\n", stats0.cacheMisses);

    printf("Prefetch 1\n"); //with prefetch
    printf("Memory reads: %d\n", stats1.memoryReads);
    printf("Memory writes: %d\n", stats1.memoryWrites);
    printf("Cache hits: %d\n", stats1.cacheHits);
    printf("Cache misses: %d\n", stats1.cacheMisses);

    freeCache(cache0);
    freeCache(cache1);

    return 0;
}
