#include "Ssd.h"
#include <cstdint>   // For uint8_t
#include <stdexcept> 
#include "defs.h"
#include<stdio.h>
#include <iostream>
#include "Table.h"
#define cout outTrace

using namespace std;
Ssd::Ssd(const char* filename, size_t size, size_t pageSize, ofstream  &outputStream) :
	_size(size), _pageSize(pageSize), _sizeOccupied(0), _readCount(0),
	_writeCount(0), _blockSize(pageSize), outTrace(outputStream) {
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



int Ssd::writeData(const void* data, size_t seek) {
    TRACE(true, outTrace);

    if (seek + _blockSize > _size) {
        cout << "Found less size to write, hence exiting.Offset seek:" << seek << " PageSize:" << _blockSize << " Size of storage" << _size << endl;  
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

int Ssd::writeData(Table data, size_t offset)
{
    TRACE(true, outTrace);
    if (offset + _pageSize > _size) {
        cout << "Found less size to write, hence exiting.Offset:" << offset << " PageSize:" << _pageSize << " Size of storage" << _size <<  endl;  
        return FEOF;
    }
    cout << "Offset :" << offset << " PageSize:" << _pageSize << " Size of storage " << _size << endl;  
    fseek(filePtr, offset, SEEK_SET);
	// size_t numBlockToWrite = (data_size/_pageSize) + ((data_size%_pageSize==1));
    // size_t written = 0;
    // cout << "Blocks to write " << numBlockToWrite << " Seek pointer" << seek << endl;
    // for (size_t i = 0; i < numBlockToWrite; i++) {
	// 	written += fwrite(data, 1, data_size, filePtr);
	// }
    // if (written != data_size) {
    //     return FIO;
    // }
    // _writeCount = _writeCount + (data_size/_pageSize) + ((data_size%_pageSize==1));
    // return SUCCESS;
    size_t written = 0;
    cout << data._NumRows << " " << data._rowSize << endl;
    for(size_t i = 0; i < data._NumRows; i++)
	{
		//cout << "Row number " << i  << " " << offset + i*(row_size) << " " << row_size << endl;
		// hdd->writeData(static_cast<const void*>(tmp[i]),offset + i*(row_size),row_size);
        written += fwrite(data[i], 1, data._rowSize, filePtr);
	}
    if (written != data._rowSize) {
        cout << "Written data not equal to rowSize";
        //return FIO;
    }
    //_writeCount = _writeCount + (data_size/_pageSize) + ((data_size%_pageSize==1));
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
