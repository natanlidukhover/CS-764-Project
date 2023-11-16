#include "Table.h"
#include "Dram.h"
#include <cstdlib>

Table::Table(size_t NumRows, size_t NumCols, size_t RecordSize,
	size_t SortKey):_NumRows(NumRows), _NumCols(NumCols),
	_RecordSize(RecordSize), _SortKey(SortKey)
{
	_rowSize = RecordSize * NumCols;
	_table = (uint8_t **)malloc(sizeof(uint8_t *) * NumRows);
	for (size_t i = 0; i < NumRows; i++) {
		_table[i] = (uint8_t *)dram.getSpace(_rowSize);
	}
}

Table::~Table()
{
	_rowSize = _RecordSize * _NumCols;
	for (int i = _NumRows - 1; i >= 0; i--) {
		dram.freeSpace(_table[i], _rowSize);
	}
	free(_table);
}

uint8_t *Table::operator[](size_t row) {
	return _table[row];
}

uint8_t& Table::operator()(size_t row, size_t col) {
	return _table[row][col];
}

uint8_t Table::operator()(size_t row, size_t col) const {
	return _table[row][col];
}

uint8_t **Table::operator()(size_t row) {
	return &(_table[row]);
}
