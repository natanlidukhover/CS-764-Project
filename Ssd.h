#ifndef SSD_H
#define SSD_H

#include <cstdio>
#include <cstdint>  // For uint8_t

class Ssd {
public:
    Ssd(const char* filename, size_t size, size_t pageSize, size_t data_size);
    ~Ssd();

    // Updated method signatures
    bool writeData(uint8_t data);
    bool readData(uint8_t* buffer, size_t offset, size_t numPages);

    int writeData(const void* data, size_t seek, size_t data_size);
    int readData(void* buffer, size_t offset);
    size_t _blockSize;   // most efficient IO unit
    size_t _dataSize; // size of data in bytes
    size_t getReadCount();
    size_t getWriteCount();

private:
    FILE* filePtr;
    size_t _size; //size of the storage source (hdd,ssd,etc)
    size_t _pageSize;
    size_t _sizeOccupied;

    size_t _readCount;  // read call counter
    size_t _writeCount; // write call counter
};

#endif // SSD_H
