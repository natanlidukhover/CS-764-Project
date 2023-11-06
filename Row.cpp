#include "Row.h"

#include <cstdlib>

#include "Dram.h"

Row::Row(size_t rowSize, size_t recordSize)
    : _rowSize(rowSize), _recordSize(recordSize) {
    // row = new vector<uint8_t>(rowSize);
    // We cannot do the above thing since we have to use emulated RAM
    _row = dram.getSpace(rowSize);
}

Row::~Row() {
	dram.freeSpace(_row, _rowSize);
}

unsigned short Row::operator[](size_t index) {
    // TODO Add error checking code
    return ((unsigned short*) _row)[index];
}

Table::Table(size_t numRows, size_t numCols, size_t recordSize)
    : _numRows(numRows), _numCols(numCols), _recordSize(recordSize) {
    size_t rowSize = recordSize * numCols;
    _table = (Row**) malloc(sizeof(Row*) * numRows);
    for (size_t i = 0; i < numRows; i++) {
        _table[i] = new Row(rowSize, recordSize);
    }
}

Table::~Table() {
    for (int i = _numRows - 1; i >= 0; i--) {
        delete _table[i];
    }
}

Row Table::operator[](size_t row) {
	return *(_table[row]);
}
