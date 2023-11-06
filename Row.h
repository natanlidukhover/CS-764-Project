#ifndef _ROW_H_
#define _ROW_H_

#include <cstddef>

class Row {
public:
    Row(size_t rowSize, size_t recordSize);
    ~Row();
    unsigned short operator[](size_t index);
    size_t _rowSize, _recordSize;

private:
    void* _row;
};  // class Record

class Table {
public:
    Table(size_t numRows, size_t numCols, size_t recordSize);
    ~Table();
    Row operator[](size_t row);

private:
    Row** _table;
    size_t _numRows, _numCols, _recordSize;
};

#endif  // _ROW_H_