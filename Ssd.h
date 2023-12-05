#ifndef SSD_H
#define SSD_H

#include <cstdio>
#include <cstdint>  // For uint8_t
#include "Table.h"
#include<fstream>
using namespace std;
class Ssd {
public:
    Ssd(const char* filename, size_t size, size_t blockSize);
    ~Ssd();

    int writeData(const void* data, size_t seek);
    int readData(void* buffer, size_t offset);
    size_t getReadCount();
    size_t getWriteCount();
private:
    FILE* filePtr;
    size_t _size; //size of the storage source (hdd,ssd,etc)
    size_t _sizeOccupied;

    size_t _readCount;  // read call counter
    size_t _writeCount; // write call counter
    size_t _blockSize;   // most efficient IO unit
};

#endif // SSD_H
