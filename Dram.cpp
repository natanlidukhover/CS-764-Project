#include "Dram.h"

#include <stdint.h>

Dram::Dram(size_t size) : _size(size), _sizeOccupied(0) {
    freePtr = startPtr = malloc(size);
}

Dram::~Dram() {
    free(startPtr);
}

void* Dram::getSpace(size_t size) {
    if (_sizeOccupied + size > _size) {
        return NULL;
    }
    // TODO Add exception raising code
    void* ptr = freePtr;
    freePtr = (uint8_t*) freePtr + size;
    _sizeOccupied += size;

    return ptr;
}

void* Dram::freeSpace(void* ptr, size_t size) {
    uint8_t* byte_ptr = (uint8_t*) ptr;
    if (byte_ptr + size == freePtr) {
        freePtr = (uint8_t*) freePtr - size;
        _sizeOccupied -= size;
        return byte_ptr;
    } else {
        return NULL;
    }
}

Dram dram(100 * 1024 * 1024);
