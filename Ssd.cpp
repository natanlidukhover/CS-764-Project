#include "Ssd.h"

#include <stdio.h>

#include <cstdint>  // For uint8_t
#include <iostream>
#include <stdexcept>

#include "defs.h"
#define cout outTrace

using namespace std;

Ssd::Ssd(const char* filename, size_t size, size_t blockSize) : _size(size), _sizeOccupied(0), _readCount(0), _writeCount(0), _blockSize(blockSize) {
    filePtr = fopen(filename, "w+");
    if (filePtr == nullptr) {
        throw std::runtime_error("Failed to open file");
    }
}

Ssd::~Ssd() {
    fclose(filePtr);
}

/**
 * Assume that the data size is always a multiple of _blockSize
 * @param const void* data: Write _blockSize amount of data present in pointer buffer
 * @param size_t seek: write data from buffer pointer starting at offset seek from ssd
 *
 * @return size_t: The number of bytes written. If we reach end of file, then we return 0
 */
size_t Ssd::writeData(const void* data, size_t seek) {
    if (seek + _blockSize > _size) {
        cout << "Found less size to write, hence exiting.Offset seek:" << seek << " BlockSize:" << _blockSize << " Size of storage" << _size << endl;
        return 0;
    }
    // cout << "Offset seek:" << seek << " BlockSize:" << _blockSize << " Size of storage " << _size << " Filepointer" << filePtr << endl;
    fseek(filePtr, seek, SEEK_SET);
    size_t written = fwrite(data, 1, _blockSize, filePtr);
    fflush((filePtr));
    _writeCount = _writeCount + 1;
    return written;
}

/**
 * @param void* buffer: Read _blockSize amount of data in pointer buffer
 * @param size_t seek: Read data into buffer pointer starting at offset seek from hdd
 *
 * @return size_t: The number of bytes read. If we reach end of file, then we return 0
 */
size_t Ssd::readData(void* buffer, size_t seek) {
    if (seek + _blockSize > _size) {
        return 0;
    }
    fseek(filePtr, seek, SEEK_SET);
    size_t read_bytes = fread(buffer, 1, _blockSize, filePtr);
    _readCount++;
    return read_bytes;
}

size_t Ssd::getReadCount() {
    return _readCount;
}

size_t Ssd::getWriteCount() {
    return _writeCount;
}
