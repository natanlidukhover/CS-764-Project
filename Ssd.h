#ifndef SSD_H
#define SSD_H

#include <cstdio>
#include <cstdlib>

class Ssd {
public:
    Ssd(const char* filename, size_t size, size_t blockSize);
    ~Ssd();
    int writeData(const void* data, size_t seek);
    void* readData(void* buffer, size_t offset);

    size_t getReadCount() const;
    size_t getWriteCount() const;

private:
    FILE* filePtr;
    size_t _size;
    size_t _readCount;  // read call counter
    size_t _writeCount; // write call counter
	size_t _blockSize;	// most efficient IO unit
};

#endif // SSD_H
