#ifndef SSD_H
#define SSD_H

#include <cstdio>
#include <cstdint>  // For uint8_t

class Ssd {
public:
    Ssd(const char* filename, size_t size, size_t pageSize);
    ~Ssd();

    // Updated method signatures
    bool writeData(uint8_t data);
    bool readData(uint8_t* buffer, size_t offset, size_t numPages);

    size_t getReadCount() const;
    size_t getWriteCount() const;

private:
    FILE* filePtr;
    size_t _size;
    size_t _pageSize;
    size_t _sizeOccupied;

    size_t _readCount;  // read call counter
    size_t _writeCount; // write call counter
};

#endif // SSD_H
