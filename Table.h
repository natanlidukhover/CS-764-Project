#ifndef __Table_h__
#define __Table_h__

#include <cstddef>
#include <cstdint>

class Table
{
public:
	Table(size_t NumRows, size_t NumCols, size_t RecordSize, size_t SortKey = 0);
	~Table();
	uint8_t *operator[](size_t row);
	uint8_t **operator()(size_t row);
	uint8_t operator()(size_t row, size_t col) const;
	uint8_t& operator()(size_t row, size_t col);
	uint8_t **_table;
	size_t _NumRows, _NumCols, _RecordSize, _SortKey, _rowSize;
};
#endif
