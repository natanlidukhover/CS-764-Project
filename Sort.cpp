#include "Sort.h"
#include <iostream>

// SortPlan::SortPlan (Plan * const input) : _input (input)
// {
// 	// TRACE (true);
// } // SortPlan::SortPlan

// SortPlan::~SortPlan ()
// {
// 	// TRACE (true);
// 	delete _input;
// } // SortPlan::~SortPlan

// Iterator * SortPlan::init () const
// {
// 	// TRACE (true);
// 	return new SortIterator (this);
// } // SortPlan::init

// SortIterator::SortIterator (SortPlan const * const plan) :
// 	_plan (plan), _input (plan->_input->init ()),
// 	_consumed (0), _produced (0)
// {
// 	// TRACE (true);

// 	while (_input->next ())  ++ _consumed;
// 	delete _input;

// 	traceprintf ("consumed %lu rows\n",
// 			(unsigned long) (_consumed));
// } // SortIterator::SortIterator

// SortIterator::~SortIterator ()
// {
// 	// TRACE (true);

// 	traceprintf ("produced %lu of %lu rows\n",
// 			(unsigned long) (_produced),
// 			(unsigned long) (_consumed));
// } // SortIterator::~SortIterator

// bool SortIterator::next ()
// {
// 	// TRACE (true);

// 	if (_produced >= _consumed)  return false;

// 	++ _produced;
// 	return true;
// } // SortIterator::next

int partition(uint8_t *data, int startRow, int endRow, int rowSize) {
	int pivot = endRow;
	int i = startRow - 1;
	uint8_t tmp;
	int cmp_col;
	int col = 0;

	for (int j = startRow; j <= endRow - 1; j++) {
		cmp_col = col;
		while (data[j * rowSize + cmp_col] == data[pivot * rowSize + cmp_col]) {
			cmp_col = (cmp_col + 1) % rowSize;
			if (cmp_col == col)
				break;
		}
		if (data[j * rowSize + cmp_col] < data[pivot * rowSize + cmp_col]) {
			i++;
			for (int k = 0; k < rowSize; k++) {
				tmp = data[i * rowSize + k];
				data[i * rowSize + k] = data[j * rowSize + k];
				data[j * rowSize + k] = tmp;
			}
		}
	}
	i++;
	for (int k = 0; k < rowSize; k++) {
		tmp = data[i * rowSize + k];
		data[i * rowSize + k] = data[endRow * rowSize + k];
		data[endRow * rowSize + k] = tmp;
	}
	return i;
}

void qs(uint8_t *data, size_t startRow, size_t endRow, size_t rowSize) {
	int pivot;
	if (startRow >= endRow || startRow < 0)
		return;
	pivot = partition(data, startRow, endRow, rowSize);
	qs(dataRow, startRow, pivot - 1, rowSize);
	qs(dataRow, pivot + 1, endRow, rowSize);
}

void quickSort(uint8_t *data, size_t numRows, size_t rowSize) {
	qs(data, 0, numRows - 1, rowSize);
}

void verifySortedRuns(uint8_t *data, int numRows, size_t rowSize) {
	int col = 0;
	int cmp_col;
	int startRow = 0;
	for (int i = startRow + 1; i <= endRow; i++) {
		cmp_col = col;
		while (data[i * rowSize + cmp_col] == data[(i - 1) * rowSize + cmp_col]) {
			cmp_col = (cmp_col + 1) % rowSize;
			if (cmp_col == col)
				break;
		}
		if (data[i * rowSize + cmp_col] < data[(i - 1) * rowSize + cmp_col]) {
			std::cout << "unsorted at row:" << i << std::endl;
			return;
		}
	}
}
