#include "Ssd.h"
#include <cstdint>   // For uint8_t
#include <stdexcept> 

Ssd::Ssd(const char* filename, size_t size, size_t pageSize) : _size(size), _pageSize(pageSize), _sizeOccupied(0), _readCount(0), _writeCount(0) {
    filePtr = fopen(filename, "w+");
    if (filePtr == nullptr) {
        throw std::runtime_error("Failed to open file");
    }
}

Ssd::~Ssd() {
    fclose(filePtr);
}

bool Ssd::writeData(uint8_t data) {

    if (_sizeOccupied + 1 > _size) {
        return false;
    }

    fseek(filePtr, _sizeOccupied, SEEK_SET);
    size_t written = fwrite(&data, 1, 1, filePtr);
    if (written != 1) {
        return false;
    }

    _sizeOccupied += 1;

    if (_sizeOccupied % _pageSize == 0) {
        _writeCount++;
    }

    return true;
}

bool Ssd::readData(uint8_t* buffer, size_t offset, size_t numPages) {
    uint8_t* bufferPtr = reinterpret_cast<uint8_t*>(buffer);

    for (size_t page = 0; page < numPages; ++page) {
        if (offset + _pageSize > _sizeOccupied) {
            break;
        }

        fseek(filePtr, offset, SEEK_SET);
        size_t read = fread(bufferPtr, 1, _pageSize, filePtr);
        if (read != _pageSize) {
            break;
        }

        _readCount++;
        offset += _pageSize;
        bufferPtr += _pageSize;
    }

    return true;
}
