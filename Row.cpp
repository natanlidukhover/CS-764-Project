#include "Row.h"
#include "Dram.h"
#include <cstdlib>

Row::Row(size_t RowSize, size_t RecordSize):_RowSize(RowSize),
	_RecordSize(RecordSize)
{
	// row = new vector<uint8_t>(RowSize);
	// We cannot do the above thing since we have to use emulated RAM
	_row = dram.GetSpace(RowSize);
}
Row::~Row()
{
	dram.FreeSpace(_row, _RowSize);
}
unsigned short Row::operator[](size_t index)
{
	//TODO Add error checking code
	return ((unsigned short *)_row)[index];
}

Table::Table(size_t NumRows, size_t NumCols, size_t RecordSize):_NumRows(NumRows),
	_NumCols(NumCols), _RecordSize(RecordSize)
{
	size_t RowSize = RecordSize * NumCols;
	_table = (Row **)malloc(sizeof(Row *) * NumRows);
	for (int i = 0; i < NumRows; i++) {
		_table[i] = new Row(RowSize, RecordSize);
	}
}

Table::~Table()
{
	for (int i = _NumRows - 1; i >= 0; i--) {
		delete _table[i];
	}
}

Row Table::operator[](size_t row) {
	return *(_table[row]);
}
