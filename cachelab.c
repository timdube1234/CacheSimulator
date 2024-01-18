#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

typedef struct {
    bool valid;
    int tag;
    int useFrequency; // LFU
    int lastUsedTime; // LRU
} CacheLine;

// Gets input for the cache
void getInput(int *numSets, int *linesPerSet, int *blockSize, int *memorySize, int *hitDuration, int *missPenalty, char *replacementPolicy) {
    scanf("%d", numSets);
    scanf("%d", linesPerSet);
    scanf("%d", blockSize);
    scanf("%d", memorySize);
    scanf("%s", replacementPolicy);
    scanf("%d", hitDuration);
    scanf("%d", missPenalty);
}

// Processes each memory address
void processAddress(unsigned int address, CacheLine **cache, int numSets, int linesPerSet, int blockOffsetBits, int setIndexBits, int tagBits, int *missCount, int *totalCycles, int hitDuration, int missPenalty, char *replacementPolicy, int currentTime) {
    unsigned int blockOffsetMask = (1 << blockOffsetBits) - 1;
    unsigned int setIndexMask = (1 << setIndexBits) - 1;

    unsigned int blockOffset = address & blockOffsetMask;
    unsigned int setIndex = (address >> blockOffsetBits) & setIndexMask;
    unsigned int tag = address >> (blockOffsetBits + setIndexBits);

    bool isHit = false;
    int replaceIndex = 0;
    int minMetric = INT_MAX;

    for (int i = 0; i < linesPerSet; i++) {
        if (cache[setIndex][i].valid && cache[setIndex][i].tag == tag) {
            isHit = true;
            if (strcmp(replacementPolicy, "LFU") == 0) {
                cache[setIndex][i].useFrequency++;
            } else if (strcmp(replacementPolicy, "LRU") == 0) {
                cache[setIndex][i].lastUsedTime = currentTime;
            }
            break;
        }

        int metric = (strcmp(replacementPolicy, "LFU") == 0) ? cache[setIndex][i].useFrequency : cache[setIndex][i].lastUsedTime;
        if (metric < minMetric) {
            replaceIndex = i;
            minMetric = metric;
        }
    }

    if (!isHit) {
        *totalCycles += (hitDuration + missPenalty);
        *missCount += 1;
        cache[setIndex][replaceIndex].valid = true;
        cache[setIndex][replaceIndex].tag = tag;
        if (strcmp(replacementPolicy, "LFU") == 0) {
            cache[setIndex][replaceIndex].useFrequency = 1;
        } else if (strcmp(replacementPolicy, "LRU") == 0) {
            cache[setIndex][replaceIndex].lastUsedTime = currentTime;
        }
    } else {
        *totalCycles += hitDuration;
    }

    printf("%X %s", address, isHit ? "H\n" : "M\n");
}

int main() {
    int numSets, linesPerSet, blockSize, memorySize, hitDuration, missPenalty;
    char replacementPolicy[4];  
    getInput(&numSets, &linesPerSet, &blockSize, &memorySize, &hitDuration, &missPenalty, replacementPolicy);

    int blockOffsetBits = (int)log2(blockSize);
    int setIndexBits = (int)log2(numSets);
    int tagBits = memorySize - (blockOffsetBits + setIndexBits);

    CacheLine **cache = malloc(numSets * sizeof(CacheLine *));
    for (int i = 0; i < numSets; i++) {
        cache[i] = malloc(linesPerSet * sizeof(CacheLine));
        for (int j = 0; j < linesPerSet; j++) {
            cache[i][j].valid = false;
            cache[i][j].tag = -1;
            cache[i][j].useFrequency = 0;
            cache[i][j].lastUsedTime = 0;
        }
    }

    int address, missCount = 0, totalCycles = 0, accessCount = 0, currentTime = 0;
    while (scanf("%x", &address) == 1 && address != -1) {
        processAddress(address, cache, numSets, linesPerSet, blockOffsetBits, setIndexBits, tagBits, &missCount, &totalCycles, hitDuration, missPenalty, replacementPolicy, currentTime);
        accessCount++;
        currentTime++;
    }

    double missRate = (accessCount > 0) ? (double)missCount / accessCount : 0.0;
    printf("Miss Rate: %d%%, Total Cycles: %d\n", (int)(missRate * 100), totalCycles);

    for (int i = 0; i < numSets; i++) {
        free(cache[i]);
    }
    free(cache);

    return 0;
}
