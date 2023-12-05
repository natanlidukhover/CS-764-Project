#include "Ssd.h"
#include <cstdint>   // For uint8_t
#include <stdexcept> 
#include "defs.h"
#include<stdio.h>
#include <iostream>
#include "Table.h"
#define cout outTrace

using namespace std;
Ssd::Ssd(const char* filename, size_t size, size_t blockSize, ofstream  &outputStream) :
	_size(size), _sizeOccupied(0), _readCount(0),
	_writeCount(0), _blockSize(blockSize), outTrace(outputStream) {
    TRACE(true, outTrace);
    filePtr = fopen(filename, "a+");
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

    if (_sizeOccupied % _blockSize == 0) {
        _writeCount++;
    }

    return true;
}


int Ssd::writeData(const void* data, size_t seek) {
    TRACE(true, outTrace);

    if (seek + _blockSize > _size) {
        cout << "Found less size to write, hence exiting.Offset seek:" << seek << " blockSize:" << _blockSize << " Size of storage" << _size << endl;  
        return FEOF;
    }
    cout << "Offset seek:" << seek << " BlockSize:" << _blockSize << " Size of storage " << _size << endl;  
    fseek(filePtr, seek, SEEK_SET);
    size_t written = 0;
	written += fwrite(data, 1, _blockSize, filePtr);
	
    if (written != _blockSize) {
        return FIO;
    }
    _writeCount = _writeCount + 1;
    return SUCCESS;
}


/**
* @param buffer: Read _blockSize amount of data in pointer buffer
* @param seek: Read data into buffer pointer starting at offset seek from hdd
* @returns The number of bytes read. If we reach end of file, then we return 0
*/
int Ssd::readData(void* buffer, size_t seek) {
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
