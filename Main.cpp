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
#define KB (size_t) 1024
#define MB 1024 * KB
#define GB 1024 * MB
#define FALSE 0
#define TRUE 1

using namespace std;
std::ofstream  outTrace;

const size_t ssdBlockSize = 10 * KB;
const size_t hddBlockSize = 1 * MB;
const size_t cacheLimit = 1 * MB;
const size_t dramLimit = 100 * MB;
const size_t ssdLimit = 10 * GB;
const size_t ssdMaxRunCount = 100;
const size_t hddMaxRunCount = 97;
const size_t ssdMaxInputBufferSize = ssdMaxRunCount * ssdBlockSize;
const size_t hddMaxInputBufferSize = hddMaxRunCount * hddBlockSize;
const size_t cacheRunSize = cacheLimit - ssdBlockSize;
size_t totalDataSize;
Ssd* inputHdd;
Ssd* outputSsd = new Ssd("./output/ssd.bin", 10 * GB, 10 * KB);
Ssd* outputHdd = new Ssd("./output/hdd.bin", 900 * GB, 1 * MB);

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 * @param const int isOutputSsd: true if output storage device is an SSD, false otherwise
 * @param Ssd* outputDevice: pointer to Ssd object of output storage device
 * 
 * @return size_t: offset of next block not read yet
 * 
 * The function reads data from the input storage device into a DRAM buffer,
 * generates cache-size runs over this input, sorts these cache-size runs, merges
 * the runs together, and pushes the sorted output to the given storage device through an output buffer
*/
size_t sortDramAndStore(const size_t bytesToFill, const size_t inputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    // Will use don't remove:
    //const size_t runSize = cacheRunSize;
    //const size_t numberOfRuns = (bytesToFill / runSize) + (bytesToFill % runSize != 0); // Round up
    const size_t outputBufferSize = outputBlockSize;
    const size_t inputBufferSize = dramLimit - outputBufferSize;

    // Create buffers
    uint8_t* inputBuffer = (uint8_t*) dram.getSpace(inputBufferSize);
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read into DRAM input buffer from the given input storage device starting at inputStartOffset
    size_t inputOffset = inputStartOffset;
    size_t bytesRead = 0;
    while (bytesRead < bytesToFill) {
        if (inputHdd->readData(inputBuffer, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
    // Generate cache-size sorted runs over the input buffer

    // Merge these runs using Tree-of-Losers and output to the given output storage device

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    dram.freeSpace(inputBuffer, inputBufferSize);

    return inputOffset;
}

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 * @param const int isOutputSsd: true if output storage device is an SSD, false otherwise
 * @param Ssd* outputDevice: pointer to Ssd object of output storage device
 * 
 * @return size_t: offset of next block not read yet
 * 
 * The function reads data from the input storage device, optimally uses DRAM to sort runs of these 
 * bytes and stores the sorted runs on SSD, then merges the sorted SSD runs to the specified
 * input storage device
*/
size_t sortSsdAndStore(const size_t bytesToFill, const size_t inputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    const size_t runSize = dramLimit - inputBlockSize;
    const size_t outputBufferSize = outputBlockSize;
    const size_t minDramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize;
    const size_t ssdMaxDataSize = bytesToFill - minDramInputBufferSize;
    const size_t numberOfSsdRuns = (ssdMaxDataSize / runSize) + (ssdMaxDataSize % runSize != 0); // Round up
    const size_t dramInputBufferSize = dramLimit - outputBufferSize - (numberOfSsdRuns * ssdBlockSize);
    const size_t ssdActualDataSize = bytesToFill - dramInputBufferSize;

    // Perform input-to-SSD sort and merge through DRAM
    size_t inputOffset = inputStartOffset;
    size_t ssdBytesLeft = ssdActualDataSize;
    while (ssdBytesLeft > 0) {
        inputOffset = sortDramAndStore(std::min(runSize, ssdBytesLeft), inputOffset, TRUE, outputSsd);
        ssdBytesLeft -= runSize;
    }

    // Create new buffers
    uint8_t* dramInputBuffer = (uint8_t*) dram.getSpace(dramInputBufferSize);
    uint8_t** ssdInputBuffers = new uint8_t*[numberOfSsdRuns];
    for (unsigned i = 0; i < numberOfSsdRuns; i++) {
        ssdInputBuffers[i] = (uint8_t*) dram.getSpace(ssdBlockSize);
    }
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read input into DRAM-to-HDD input buffer
    while (inputOffset < bytesToFill) {
        if (inputHdd->readData(dramInputBuffer, inputOffset) <= 0) {
            break;
        }
        inputOffset += inputBlockSize;
    }
    // Generate cache-size sorted runs over the input buffer

    // Merge these runs with the SSD input buffers using Tree-of-Losers and output to HDD using outputBuffer

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (unsigned i = 0; i < numberOfSsdRuns; i++) {
        (uint8_t*) dram.freeSpace(ssdInputBuffers[i], ssdBlockSize);
    }
    dram.freeSpace(dramInputBuffer, dramInputBufferSize);

    return inputOffset;
}

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 * 
 * @return size_t: offset of next block not read yet
*/
size_t sortHddAndStore(const size_t bytesToFill, const size_t inputStartOffset) {
    // Create constants
    const size_t outputBlockSize = hddBlockSize;
    const size_t inputBlockSize = hddBlockSize;
    const size_t runSize = ssdLimit - inputBlockSize;
    const size_t outputBufferSize = outputBlockSize;
    const size_t minDramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize - hddMaxInputBufferSize;
    const size_t hddMaxDataSize = bytesToFill - ssdMaxInputBufferSize - minDramInputBufferSize;
    const size_t numberOfHddRuns = (hddMaxDataSize / runSize) + (hddMaxDataSize % runSize != 0); // Round up
    const size_t dramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize - (numberOfHddRuns * hddBlockSize);
    const size_t hddActualDataSize = bytesToFill - ssdLimit - dramInputBufferSize;

    // Perform input-to-SSD-to-HDD sort
    size_t inputOffset = inputStartOffset;
    size_t hddBytesLeft = hddActualDataSize;
    while (hddBytesLeft > 0) {
        inputOffset = sortSsdAndStore(std::min(runSize, hddBytesLeft), inputOffset, FALSE, outputHdd);
        hddBytesLeft -= runSize;
    }

    // Perform input-to-SSD sort
    inputOffset = sortSsdAndStore(ssdLimit, inputOffset, TRUE, outputSsd);

    // Create buffers
    uint8_t* dramInputBuffer = (uint8_t*) dram.getSpace(dramInputBufferSize);
    uint8_t** ssdInputBuffers = new uint8_t*[ssdMaxRunCount];
    for (unsigned i = 0; i < ssdMaxRunCount; i++) {
        ssdInputBuffers[i] = (uint8_t*) dram.getSpace(ssdBlockSize);
    }
    uint8_t** hddInputBuffers = new uint8_t*[numberOfHddRuns];
    for (unsigned i = 0; i < numberOfHddRuns; i++) {
        hddInputBuffers[i] = (uint8_t*) dram.getSpace(hddBlockSize);
    }
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read input into DRAM-to-HDD input buffer
    while (inputOffset < bytesToFill) {
        if (inputHdd->readData(dramInputBuffer, inputOffset) <= 0) {
            break;
        }
        inputOffset += inputBlockSize;
    }
    // Generate cache-size runs over the input buffer

    // Merge these runs with the SSD and HDD input buffers using Tree-of-Losers and output to HDD using hddOutputBuffer

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (unsigned i = 0; i < numberOfHddRuns; i++) {
        (uint8_t*) dram.freeSpace(hddInputBuffers[i], hddBlockSize);
    }
    for (unsigned i = 0; i < ssdMaxRunCount; i++) {
        (uint8_t*) dram.freeSpace(ssdInputBuffers[i], ssdBlockSize);
    }
    dram.freeSpace(dramInputBuffer, dramInputBufferSize);

    return inputOffset; 
}

int main_buggy(int argc, char* argv[]) {

    // Default values
   	size_t number_of_records = 12;							// Number of rows
	size_t row_size = 3; 									// Size of each row in bytes
    size_t totalDataSize = number_of_records * row_size;   	// Total amount of data in bytes
    size_t blockSize = 3;
    std::string o_filename="o.txt";							// Output file name
    inputHdd = new Ssd("./input/testData.bin", totalDataSize, 1 * MB);

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
    Ssd *  unsorted_hdd = new Ssd("./input/testData.bin",(size_t)number_of_records*row_size, 3);
    Ssd *  sorted_hdd = new Ssd("./output/testData.bin",(size_t)number_of_records*row_size, 3);

	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (number_of_records));
	vector<int> numbers = sc_it->run();
	
	//Table tmp(number_of_records, 3, row_size/3);
	uint8_t *data = (uint8_t *)dram.getSpace(1 * pow(10, 6));
    //Run initRun(unsorted_hdd, data, (size_t)(1 * 1e6), (size_t)0, number_of_records * row_size, number_of_records * row_size, (size_t)1e6, row_size, (size_t)0);
    //uint8_t *row;
	for (size_t i = 0; i < number_of_records * row_size; i += pow(10, 6)) {
		unsorted_hdd->readData(data + i, i);
	}
	for (size_t i = 0; i < number_of_records; i++) {
        //initRun.getNext(&row);
		for (size_t j = 0; j < row_size; j++) {
            //tmp[i][j] = row[j];
			cout << data[i * row_size + j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	quickSort(data, (int) number_of_records, (int)row_size);
	verifySortedRuns(data, (int)number_of_records,(int) row_size);

	cout << "Sorted table" << "\n";
	for (size_t i = 0; i < number_of_records; i++) {
		for (size_t j = 0; j < row_size; j++) {
			cout << (int)data[i * row_size + j] << " ";
		}
		cout << "\n";
	}
	//for hdd blockSize is given by bandwidth * latency = 100*0.01
	
	size_t offset = 0;
	for(size_t i = 0; i < (number_of_records * row_size)/blockSize; i++)
	{
		sorted_hdd->writeData(static_cast<const void*>(data + i * blockSize),offset + i*(blockSize));
	}

    /**
     * Case 1: < 100 MB
     * Case 2: < 10 GB
     * Case 3: >= 10 GB
    */
    if (totalDataSize < dramLimit) { // Case 1: Unsorted HDD->DRAM->HDD Sorted
        sortDramAndStore(totalDataSize, 0, FALSE, outputHdd);
    } else if (totalDataSize < ssdLimit) { // Case 2: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted
        sortSsdAndStore(totalDataSize, 0, FALSE, outputHdd);
    } else { // Case 3: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted, Sorted DRAM + Sorted SSD + Sorted HDD->HDD Sorted
        sortHddAndStore(totalDataSize, 0);
    }
    return 0;
}  // main
