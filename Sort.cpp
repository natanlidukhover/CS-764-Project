#include "Sort.h"

#include <iostream>

#include "defs.h"
#define cout outTrace

int partition(uint8_t* data, int startRow, int endRow, int rowSize) {
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

void qs(uint8_t* data, int startRow, int endRow, int rowSize) {
    int pivot;
    if (startRow >= endRow || startRow < 0)
        return;
    pivot = partition(data, startRow, endRow, rowSize);

    qs(data, startRow, pivot - 1, rowSize);
    qs(data, pivot + 1, endRow, rowSize);
}

void quickSort(uint8_t* data, int numRows, int rowSize) {
    qs(data, 0, numRows - 1, rowSize);
}

void verifySortedRuns(uint8_t* data, int endRow, int rowSize) {
    int col = 0;
    int cmp_col;
    int startRow = 0;
    for (int i = startRow + 1; i < endRow; i++) {
        cmp_col = col;
        while (data[i * rowSize + cmp_col] == data[(i - 1) * rowSize + cmp_col]) {
            cmp_col = (cmp_col + 1) % rowSize;
            if (cmp_col == col)
                break;
        }
        if (data[i * rowSize + cmp_col] < data[(i - 1) * rowSize + cmp_col]) {
            cout << "unsorted at row:" << i << std::endl;
            return;
        }
    }
    cout << "Verification of data successful!" << endl;
}
