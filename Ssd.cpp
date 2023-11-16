#include "Ssd.h"
#include <cstdio>
#include <cstring>
#include <iostream>

constexpr size_t PAGE_SIZE = 8192; //8KB of data

class Ssd {
public:
    Ssd(const char* filename, size_t size);
    ~Ssd();
    void* writeData(const void* data);
    void* readData(void* buffer, size_t offset);

    size_t getReadCount() const { return _readCount; }
    size_t getWriteCount() const { return _writeCount; }

private:
    FILE* filePtr;
    size_t _size;
    size_t _sizeOccupied;
    size_t _readCount = 0;  // read call counter
    size_t _writeCount = 0; // write call counter
};

Ssd::Ssd(const char* filename, size_t size) : _size(size), _sizeOccupied(0), _readCount(0), _writeCount(0) {
    filePtr = fopen(filename, "w+");
    if (filePtr == nullptr) {
        throw std::runtime_error("Failed to open file");
    }
}

Ssd::~Ssd() {
    fclose(filePtr);
}

void* Ssd::writeData(const void* data) {
    if (_sizeOccupied + PAGE_SIZE > _size) {
        return nullptr;
    }
    fseek(filePtr, _sizeOccupied, SEEK_SET);
    size_t written = fwrite(data, 1, PAGE_SIZE, filePtr);
    if (written != PAGE_SIZE) {
        return nullptr;
    }
    _sizeOccupied += PAGE_SIZE;
    _writeCount++;  
    return (void*)(_sizeOccupied - PAGE_SIZE);
}

void* Ssd::readData(void* buffer, size_t offset) {
    if (offset + PAGE_SIZE > _sizeOccupied) {
        return nullptr;
    }
    fseek(filePtr, offset, SEEK_SET);
    size_t read = fread(buffer, 1, PAGE_SIZE, filePtr);
    if (read != PAGE_SIZE) {
        return nullptr;
    }
    _readCount++;
    return buffer;
}
