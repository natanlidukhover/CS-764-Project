#include "Iterator.h"

int partition(uint8_t* data, int startRow, int endRow, int rowSize);
void qs(uint8_t* data, int startRow, int endRow, int rowSize);
void quickSort(uint8_t* data, int numRows, int rowSize);
void verifySortedRuns(uint8_t* data, int numRows, int rowSize);
