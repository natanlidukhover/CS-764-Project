#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "tol.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>
#include "Ssd.h"
#include <fstream>
#include <math.h>
#define cout outTrace
#define KB 1024
#define MB 1024 * KB
#define GB 1024 * MB

using namespace std;
std::ofstream  outTrace;

int main_buggy(int argc, char* argv[]) {

    // Default values
   	size_t number_of_records = 5;							// Number of rows
	size_t row_size = 1; 									// Size of each row in bytes
    size_t totalDataSize = number_of_records * row_size;   	// Total amount of data in bytes
    std::string o_filename="o.txt";							// Output file name

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
	outTrace.open(o_filename,  std::ios_base::out );
    TRACE(true);

 	// Print the values
    cout << "Total number of records " << number_of_records << std::endl;
    cout << "Size of one record" << row_size << std::endl;
    cout << "Output_filename: " << o_filename << std::endl;
    Ssd *  hdd = new Ssd("./input/testData.bin",(size_t)number_of_records*row_size, pow(10,6));

	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (number_of_records,outTrace));
	vector<int> numbers = sc_it->run();
	
	Table tmp(number_of_records, 3, row_size/3);
	uint8_t *data = (uint8_t *)dram.getSpace(1 * pow(10, 6));
    Run initRun(hdd, data, (size_t)(1 * 1e6), (size_t)0, number_of_records * row_size, number_of_records * row_size, (size_t)1e6, row_size, (size_t)0);
    uint8_t *row;
    size_t k = 0;
	for (size_t i = 0; i < number_of_records; i++) {
        initRun.getNext(&row);
		for (size_t j = 0; j < row_size; j++) {
            tmp[i][j] = row[j];
			cout << tmp[i][j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int sort_col = 0;
	generateRuns(tmp, 0, number_of_records - 1, sort_col);
	verifySortedRuns(tmp, 0, number_of_records - 1, sort_col);

	cout << "Sorted table" << "\n";
	for (size_t i = 0; i < number_of_records; i++) {
		for (size_t j = 0; j < row_size; j++) {
			cout << (int)tmp[i][j] << " ";
		}
		cout << "\n";
	}
	//for hdd blockSize is given by bandwidth * latency = 100*0.01
	
	size_t offset = (number_of_records*row_size) + 1;
	for(size_t i = 0; i < number_of_records; i++)
	{
		hdd->writeData(static_cast<const void*>(tmp[i]),offset + i*(row_size));
	}

/**
     * Case 1: <= 99 MB
     * Case 2: <= 9.9 GB
     * Case 3: <= 990 GB
    */
    // Constant creation
    const size_t ssdBlockSize = 10 * KB;
    const size_t hddBlockSize = 1 * MB;
    const size_t cacheLimit = 1 * MB;
    const size_t dramLimit = 100 * MB;
    const size_t ssdLimit = 10 * GB;
    const size_t ssdMaxRunCount = 100;
    const size_t hddMaxRunCount = 97;
    const size_t cacheRunSize = 990 * KB;
    size_t dramToSsdRunSize = dramLimit - ssdBlockSize;
    size_t dramToHddInputBufferSize = dramLimit - hddBlockSize;
    // Buffer creation
    uint8_t* dramToHddInputBuffer = (uint8_t*) malloc(dramToHddInputBufferSize);
    uint8_t* hddOutputBuffer = (uint8_t*) malloc(hddBlockSize);

    // Case 1 (exclusive with case 2/3)
    // Read input into DRAM to HDD input buffer
    
    // Generate cache-size runs over the input buffer

    // Merge these runs using Tree-of-Losers and output to HDD using hddOutputBuffer

    // Case 2
    // Constant creation
    dramToHddInputBufferSize = dramLimit - hddBlockSize - (ssdMaxRunCount * ssdBlockSize);
    size_t ssdTotalDataSize = totalDataSize - dramToHddInputBufferSize;
    // Bufer creation
    dramToHddInputBuffer = (uint8_t*) malloc(dramToHddInputBufferSize);
    uint8_t* dramToSsdInputBuffer = (uint8_t*) malloc(dramToSsdRunSize);
    uint8_t* ssdOutputBuffer = (uint8_t*) malloc(ssdBlockSize);
    uint8_t** ssdInputBuffers = (uint8_t**) malloc(ssdBlockSize * ssdMaxRunCount);

    // Read input into DRAM to SSD input buffer

    // Generate cache-size runs over the input buffer

    // Merge these runs using Tree-of-Losers and output to SSD using ssdOutputBuffer

    // Read input into DRAM to HDD input buffer

    // Generate cache-size runs over the input buffer

    // Merge these runs with the SSD input buffers using Tree-of-Losers and output to HDD using hddOutputBuffer


    // Case 3 (in addition to case 2)
    // Constant creation
    dramToHddInputBufferSize = dramLimit - hddBlockSize - (ssdMaxRunCount * ssdBlockSize) - (hddMaxRunCount * hddBlockSize);
    // Buffer creation
    dramToHddInputBuffer = (uint8_t*) malloc(dramToHddInputBufferSize);
    uint8_t** hddInputBuffers = (uint8_t**) malloc(hddBlockSize * hddMaxRunCount);

    // Read input into the DRAM to HDD input buffer

    // Generate cache-size runs over the input buffer

    // Merge these runs with the SSD and HDD input buffers using Tree-of-Losers and output to HDD using hddOutputBuffer


    return 0;
}  // main
