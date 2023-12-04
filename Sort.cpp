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

int partition(Table& t, int startRow, int endRow, int col) {
	int pivot = endRow;
	int i = startRow - 1;
	uint8_t *tmp;
	int cmp_col;

	for (int j = startRow; j <= endRow - 1; j++) {
		cmp_col = col;
		while (t(j, cmp_col) == t(pivot, cmp_col)) {
			cmp_col = (cmp_col + 1) % t._rowSize;
			if (cmp_col == col)
				break;
		}
		if (t(j, cmp_col) < t(pivot, cmp_col)) {
			i++;
			tmp = *t(i);
			*t(i) = *t(j);
			*t(j) = tmp;
		}
	}
	i++;
	tmp = *t(i);
	*t(i) = *t(endRow);
	*t(endRow) = tmp;
	return i;
}

void generateRuns(Table& t, int startRow, int endRow, int col) {
	int pivot;
	if (startRow >= endRow || startRow < 0)
		return;
	pivot = partition(t, startRow, endRow, col);
	generateRuns(t, startRow, pivot - 1, col);
	generateRuns(t, pivot + 1, endRow, col);
}

void verifySortedRuns(Table &t, int startRow, int endRow, int col) {
	int cmp_col;
	for (int i = startRow + 1; i <= endRow; i++) {
		cmp_col = col;
		while (t(i, cmp_col) == t(i - 1, cmp_col)) {
			cmp_col = (cmp_col + 1) % t._rowSize;
			if (cmp_col == col)
				break;
		}
		if (t(i, cmp_col) < t(i - 1, cmp_col)) {
			std::cout << "unsorted at row:" << i << std::endl;
			return;
		}
	}
}
