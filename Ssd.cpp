#include "Ssd.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

Ssd::Ssd(size_t size) : _size(size), _sizeOccupied(0) {
    startPtr = malloc(size);
    if (startPtr == nullptr) {
        throw std::bad_alloc();
    }
}

Ssd::~Ssd() {
    free(startPtr);
}

void* Ssd::writeData(const void* data, size_t size) {
    if (_sizeOccupied + size > _size) {
        return nullptr;
    }
    void* writePtr = static_cast<uint8_t*>(startPtr) + _sizeOccupied;
    memcpy(writePtr, data, size);
    _sizeOccupied += size;

    return writePtr;
}

void* Ssd::readData(void* buffer, size_t size, size_t offset) {
    if (offset + size > _sizeOccupied) {
        return nullptr;
    }
    void* readPtr = static_cast<uint8_t*>(startPtr) + offset;
    memcpy(buffer, readPtr, size);
    return buffer;
}

