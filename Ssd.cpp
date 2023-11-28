#include "Ssd.h"

#include <cstdio>
#include <cstring>
#include <iostream>

#include "defs.h"

Ssd::Ssd(const char* filename, size_t size, size_t blockSize) : _size(size),
                                                                _readCount(0),
                                                                _writeCount(0),
                                                                _blockSize(blockSize) {
    filePtr = fopen(filename, "w+");
    if (filePtr == nullptr) {
        throw std::runtime_error("Failed to open file");
    }
}

Ssd::~Ssd() {
    fclose(filePtr);
}

int Ssd::writeData(const void* data, size_t seek) {
    if (seek + _blockSize > _size) {
        return FEOF;
    }
    fseek(filePtr, seek, SEEK_SET);
    size_t written = fwrite(data, 1, _blockSize, filePtr);
    if (written != _blockSize) {
        return FIO;
    }
    _writeCount++;
    return SUCCESS;
}

int Ssd::readData(void* buffer, size_t seek) {
    if (seek + _blockSize > _size) {
        return FEOF;
    }
    fseek(filePtr, seek, SEEK_SET);
    size_t read = fread(buffer, 1, _blockSize, filePtr);
    if (read != _blockSize) {
        return FIO;
    }
    _readCount++;
    return SUCCESS;
}

size_t Ssd::getReadCount() {
    return _readCount;
}
size_t Ssd::getWriteCount() {
    return _writeCount;
}
