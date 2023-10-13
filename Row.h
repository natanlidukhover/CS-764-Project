#include <cstddef>

class Row
{
public:
	Row(size_t RowSize, size_t RecordSize);
	~Row();
	unsigned short operator[](size_t index);
private:
	size_t _RowSize, _RecordSize;
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
