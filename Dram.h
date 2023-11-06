#ifndef _DRAM_H_
#define _DRAM_H_

#include <cstdlib>

class Dram {
public:
    Dram(size_t size);
    ~Dram();
    void* getSpace(size_t size);
    void* freeSpace(void* ptr, size_t size);

private:
    size_t _size, _sizeOccupied;
    void* startPtr;
    void* freePtr;
};

extern Dram dram;

#endif  // _DRAM_H