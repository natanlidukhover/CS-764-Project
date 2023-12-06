#include "Sort.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#define cout outTrace
void verify(const std::string &sortedFilePath, const std::string &unsortedFilePath, size_t pageSize, size_t rowSize, size_t totalUnsortedRows) {
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
    while (sortedFile.read(reinterpret_cast<char*>(buffer.data()), rowSize)) {
        std::vector<uint8_t> currentRow(buffer);

        if (!previousRow.empty() && currentRow < previousRow) {
            std::cerr << "Sorted data is not sorted correctly." << std::endl;
            return;
        }

        sortedDataSet.insert(currentRow);
        previousRow = std::move(currentRow);
    }

    // Iterate through unsorted data page by page
    size_t pageCount = 0;
    size_t rowReadCount = 0;
    while (unsortedFile.read(reinterpret_cast<char*>(buffer.data()), rowSize)) {
        std::vector<uint8_t> currentRow(buffer);

		if (sortedDataSet.find(currentRow) == sortedDataSet.end()) {
    		std::cerr << "Row not found in sorted data: ";
    		for (auto val : currentRow) {
        		std::cerr << static_cast<int>(val) << " ";
    		}
    		std::cerr << std::endl;
    		return;
}

        rowReadCount++;
        if (rowReadCount % (pageSize / rowSize) == 0) {
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
