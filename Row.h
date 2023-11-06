#ifndef _ROW_H_
#define _ROW_H_

#include <cstddef>

class Row
{
public:
	Row(size_t RowSize, size_t RecordSize);
	~Row();
	unsigned short operator[](size_t index);
	size_t _RowSize, _RecordSize;
private:
	void *_row;
}; // class Record

class Table
{
public:
	Table(size_t NumRows, size_t NumCols, size_t RecordSize);
	~Table();
	Row operator[](size_t row);
private:
	Row **_table;
	size_t _NumRows, _NumCols, _RecordSize;
};

#endif // _ROW_H_