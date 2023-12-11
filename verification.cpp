#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "Sort.h"
#define cout outTrace
void verify(const char* sortedFilePath, const char* unsortedFilePath, size_t pageSize, size_t rowSize, size_t totalUnsortedRows) {
    std::ifstream sortedFile(sortedFilePath, std::ios::binary);
    std::ifstream unsortedFile(unsortedFilePath, std::ios::binary);

    if (!sortedFile || !unsortedFile) {
        std::cerr << "Error opening files." << std::endl;
        return;
    }

    std::vector<uint8_t> buffer(rowSize);
    std::set<std::vector<uint8_t> > sortedDataSet;

    // Read and verify sorted data
    std::vector<uint8_t> previousRow;
    int row_index = 0;
    while (sortedFile.read(reinterpret_cast<char*>(buffer.data()), rowSize)) {
        std::vector<uint8_t> currentRow(buffer);

        if (!previousRow.empty() && currentRow < previousRow) {
            std::cerr << "Sorted data is not sorted correctly." << std::endl;
            std::cerr << row_index << ":" << std::endl;
            for (size_t i = 0; i < rowSize; i++) {
                std::cerr << (int) currentRow[i] << " ";
            }
            std::cerr << std::endl;
            for (size_t i = 0; i < rowSize; i++) {
                std::cerr << (int) previousRow[i] << " ";
            }
            return;
        }

        sortedDataSet.insert(currentRow);
        previousRow = std::move(currentRow);
        row_index++;
    }

    // Iterate through unsorted data page by page.
    size_t intsReadCount = 0;
    size_t rowReadCount = 0;
    size_t pageCount = 0;
    while (unsortedFile.read(reinterpret_cast<char*>(buffer.data()), rowSize)) {
        std::vector<uint8_t> unsortedRow(buffer);
        
        std::vector<uint8_t> sorted_buffer(rowSize);
        std::ifstream sortedFileRepeat(sortedFilePath, std::ios::binary);

        bool found = false;
        while(sortedFileRepeat.read(reinterpret_cast<char*>(sorted_buffer.data()), rowSize)){
            std::vector<uint8_t> currentRowSorted(sorted_buffer);
            int cnt = 0;
            for(size_t i = 0; i < rowSize; i++)
            {
                if(currentRowSorted[i] == unsortedRow[i]){
                    cnt += 1;
                    continue;
                }
                else{
                    cnt = 0;
                    break;
                }
            }
            if(cnt == rowSize) {
                found = true;
                break;
            }
        }
        if(!found){
            std::cerr << "Rowss not found in sorted data: ";
            // for (auto val : unsortedRow) {
            //     std::cerr << static_cast<int>(val) << " ";
            // }
            std::cerr << std::endl;
        }
        // if (sortedDataSet.find(currentRow) == sortedDataSet.end()) {
    //     std::cerr << "Row not found in sorted data: ";
        //     for (auto val : currentRow) {
        //         std::cerr << static_cast<int>(val) << " ";
        //     }
        //     std::cerr << std::endl;
        // }

        rowReadCount++;
        intsReadCount += rowSize;
        if (intsReadCount % pageSize == 0) {
            pageCount++;
        }

        // Check if all rows have been read
        if (rowReadCount >= totalUnsortedRows) {
            break;
        }
    }

    cout << "Verification Successful" << std::endl;
    cout << "Pages verified: " << pageCount << std::endl;
    cout << "Rows verified: " << rowReadCount << std::endl;
}
