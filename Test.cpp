#include <chrono>
#include <fstream>
#include <iostream>

#include "Dram.h"
#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Ssd.h"
#include "Table.h"
#include "defs.h"
#include "verification.h"
#define cout outTrace

using namespace std;
using namespace std::chrono;
int main1(int argc, char* argv[]) {
    auto start_main = high_resolution_clock::now();
    // Default values
    size_t number_of_records = 1e7;
    size_t row_size = 5;
    size_t blockSize = 10000;
    std::string o_filename = "o.txt";

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-c") == 0) {
            number_of_records = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-s") == 0) {
            row_size = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-o") == 0) {
            o_filename = argv[i + 1];
        }
    }
    outTrace.open(o_filename, std::ios_base::out);
    TRACE(true);

    // Print the values
    cout << "Total number of records " << number_of_records << std::endl;
    cout << "Size of one record" << row_size << std::endl;
    cout << "Output_filename: " << o_filename << std::endl;
    // for hdd blockSize is given by bandwidth * latency = 100*0.01
    Ssd* unsorted_hdd = new Ssd("./input/testData.bin", (size_t) number_of_records * row_size, blockSize);
    Ssd* sorted_hdd = new Ssd("./output/testData.bin", (size_t) number_of_records * row_size, blockSize);
    ScanIterator* const sc_it = new ScanIterator(new ScanPlan(number_of_records * row_size, blockSize));
    vector<int> numbers = sc_it->run();

    uint8_t* data = (uint8_t*) dram.getSpace(1 * pow(10, 6));
    for (size_t i = 0; i < number_of_records * row_size; i += blockSize) {
        unsorted_hdd->readData(data + i, i);
    }

    quickSort(data, number_of_records, row_size);
    verifySortedRuns(data, number_of_records, row_size);

    size_t offset = 0;
    for (size_t i = 0; i < (number_of_records * row_size) / blockSize; i++) {
        sorted_hdd->writeData(static_cast<const void*>(data + (i * blockSize)), offset + i * blockSize);
    }
    auto stop_main = high_resolution_clock::now();
    auto duration_main = duration_cast<seconds>(stop_main - start_main);

    cout << "Time taken by main for " << (number_of_records * row_size) / 1000 << "kilo bytes data "
         << duration_main.count() << " seconds" << endl;

    const char* outputFileName = "./output/testData.bin";
    const char* inputFileName = "./input/testData.bin";
    verify(outputFileName, inputFileName, blockSize, row_size, number_of_records);

    return 0;
}  // main
