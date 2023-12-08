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
#define KB (size_t) 1000
#define MB 1000 * KB
#define GB 1000 * MB
#define FALSE 0
#define TRUE 1

using namespace std;
std::ofstream  outTrace;

const size_t ssdBlockSize = 10 * KB;
const size_t hddBlockSize = 1 * MB;
const size_t cacheLimit = 1 * MB;
const size_t dramLimit = 100 * MB;
const size_t ssdLimit = 10 * GB;
const size_t hddLimit = 900 * GB;
const size_t ssdMaxRunCount = 100;
const size_t hddMaxRunCount = 97;
const size_t ssdMaxInputBufferSize = ssdMaxRunCount * ssdBlockSize;
const size_t hddMaxInputBufferSize = hddMaxRunCount * hddBlockSize;
const size_t cacheRunSize = 500 * KB;  	
size_t numberOfRecords = 2.5 * MB;    // Number of rows
size_t rowSize = 50;             // Size of each row in bytes
size_t totalDataSize = numberOfRecords * rowSize;
Ssd* inputHdd;
Ssd* outputSsd = new Ssd("./output/ssd.bin", ssdLimit, ssdBlockSize);
Ssd* outputHdd = new Ssd("./output/hdd.bin", hddLimit, hddBlockSize);

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
size_t sortDramAndStore(const size_t bytesToFill, const size_t inputStartOffset, const size_t outputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    const size_t runSize = cacheRunSize;
    const size_t numberOfRuns = (bytesToFill / runSize) + (bytesToFill % runSize != 0); // Round up
    const size_t outputBufferSize = outputBlockSize;
    const size_t inputBufferSize = dramLimit - outputBufferSize;

    // Create buffers
    uint8_t* inputBuffer = (uint8_t*) dram.getSpace(inputBufferSize);
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read into DRAM input buffer from the given input storage device starting at inputStartOffset
    size_t inputOffset = inputStartOffset;
    size_t bytesRead = 0;
    while (inputOffset < inputStartOffset + bytesToFill) {
        if (inputHdd->readData(inputBuffer + bytesRead, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
    
    // Generate cache-size runs over the input buffer and sort them
    Run** runs = new Run*[numberOfRuns];
    for (size_t i = 0; i < numberOfRuns; i++) {
        size_t minRunSize = std::min(runSize, bytesToFill - (inputStartOffset + (i * runSize)));
        runs[i] = new Run(inputHdd, inputBuffer + (i * runSize), inputBlockSize, 0, 0, minRunSize, minRunSize, rowSize, minRunSize);
        quickSort(inputBuffer + (i * runSize), runSize / rowSize, rowSize);
	    verifySortedRuns(inputBuffer + (i * runSize), runSize / rowSize, rowSize);
    }
    Run* outputRun = new Run(outputDevice, outputBuffer, outputBlockSize, outputStartOffset, 0, bytesToFill, outputBufferSize, rowSize, 0);

    // Merge these runs using Tree-of-Losers and output to the given output storage device
    ETable* t = new ETable(numberOfRecords, rowSize, rowSize, 0);
    TOL* tol = new TOL(numberOfRuns, runs, outputRun, *t);
    for (size_t i = 0; i < numberOfRecords + 1; i++) {
        tol->pass();
    }
    outputRun->flush();
    for (size_t i = 0; i < numberOfRecords; i++) {
		uint8_t *ptr;
		outputRun->getNext(&ptr);
		cout << "[" << i + 1 << "]";
		for (size_t j = 0; j < rowSize; j++) {
			cout << (int) ptr[j] << " ";
		}
		cout << endl;
	}

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
size_t sortSsdAndStore(const size_t bytesToFill, const size_t inputStartOffset, const size_t outputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    const size_t ssdRunSize = dramLimit - inputBlockSize;
    const size_t outputBufferSize = outputBlockSize;
    const size_t minDramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize;
    const size_t ssdMaxDataSize = bytesToFill - minDramInputBufferSize;
    const size_t numberOfSsdRuns = (ssdMaxDataSize / ssdRunSize) + (ssdMaxDataSize % ssdRunSize != 0); // Round up
    const size_t dramInputBufferSize = dramLimit - outputBufferSize - (numberOfSsdRuns * ssdBlockSize);
    const size_t ssdActualDataSize = bytesToFill - dramInputBufferSize;
    const size_t numberOfCacheRuns = (dramInputBufferSize / cacheRunSize) + (dramInputBufferSize % cacheRunSize != 0); // Round up
    const size_t cacheLastRunSize = dramInputBufferSize - ((numberOfCacheRuns - 1) * cacheRunSize);
    const size_t ssdLastRunSize = ssdActualDataSize - ((numberOfSsdRuns - 1) * ssdRunSize);

    // Perform input-to-SSD sort and merge through DRAM
    size_t inputOffset = inputStartOffset;
    size_t ssdBytesLeft = ssdActualDataSize;
    while (ssdBytesLeft > 0) {
        size_t minRunSize = std::min(ssdRunSize, ssdBytesLeft);
        inputOffset = sortDramAndStore(minRunSize, inputOffset, ssdActualDataSize - minRunSize, TRUE, outputSsd);
        ssdBytesLeft -= minRunSize;
    }

    // Create new buffers
    uint8_t* dramInputBuffer = (uint8_t*) dram.getSpace(dramInputBufferSize);
    uint8_t** ssdInputBuffers = new uint8_t*[numberOfSsdRuns];
    for (size_t i = 0; i < numberOfSsdRuns; i++) {
        ssdInputBuffers[i] = (uint8_t*) dram.getSpace(ssdBlockSize);
    }
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read input into DRAM-to-HDD input buffer
    size_t bytesRead = 0;
    while (inputOffset < bytesToFill) {
        if (inputHdd->readData(dramInputBuffer + bytesRead, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
    // Generate cache-size sorted runs over the input buffer
    Run** runs = new Run*[numberOfCacheRuns + numberOfSsdRuns];
    for (size_t i = 0; i < numberOfCacheRuns; i++) {
        size_t minRunSize = cacheRunSize;
        if (i == numberOfCacheRuns - 1) {
            minRunSize = cacheLastRunSize;
        }
        runs[i] = new Run(inputHdd, dramInputBuffer + (i * cacheRunSize), inputBlockSize, 0, 0, minRunSize, minRunSize, rowSize, minRunSize);
        quickSort(dramInputBuffer + (i * cacheRunSize), cacheRunSize / rowSize, rowSize);
	    verifySortedRuns(dramInputBuffer + (i * cacheRunSize), cacheRunSize / rowSize, rowSize);
    }
    for (size_t i = 0; i < numberOfSsdRuns; i++) {
        size_t minRunSize = ssdRunSize;
        if (i == numberOfSsdRuns - 1) {
            minRunSize = ssdLastRunSize;
        }
        //runs[i] = new Run(inputHdd, inputBuffer + (i * minRunSize), inputBlockSize, inputStartOffset + (i * minRunSize), 0, minRunSize, minRunSize, rowSize, minRunSize);
        runs[numberOfCacheRuns + i] = new Run(outputSsd, ssdInputBuffers[i], ssdBlockSize, i * minRunSize, minRunSize, minRunSize, ssdBlockSize, rowSize, 0);
    }
    Run* outputRun = new Run(outputDevice, outputBuffer, outputBlockSize, outputStartOffset, 0, bytesToFill, outputBufferSize, rowSize, 0);

    // Merge these runs with the SSD input buffers using Tree-of-Losers and output to HDD using outputBuffer
    ETable* t = new ETable(numberOfRecords, rowSize, rowSize, 0);
    TOL* tol = new TOL(numberOfCacheRuns + numberOfSsdRuns, runs, outputRun, *t);
    for (size_t i = 0; i < numberOfRecords + 1; i++) {
        tol->pass();
    }
    outputRun->flush();
    for (size_t i = 0; i < numberOfRecords; i++) {
		uint8_t *ptr;
		outputRun->getNext(&ptr);
		cout << "[" << i + 1 << "]";
		for (size_t j = 0; j < rowSize; j++) {
			cout << (int) ptr[j] << " ";
		}
		cout << endl;
	}

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (size_t i = 0; i < numberOfSsdRuns; i++) {
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
        //inputOffset = sortSsdAndStore(std::min(runSize, hddBytesLeft), inputOffset, hddActualDataSize - , FALSE, outputHdd);
        hddBytesLeft -= runSize;
    }

    // Perform input-to-SSD sort
    //inputOffset = sortSsdAndStore(ssdLimit, inputOffset, TRUE, outputSsd);

    // Create buffers
    uint8_t* dramInputBuffer = (uint8_t*) dram.getSpace(dramInputBufferSize);
    uint8_t** ssdInputBuffers = new uint8_t*[ssdMaxRunCount];
    for (size_t i = 0; i < ssdMaxRunCount; i++) {
        ssdInputBuffers[i] = (uint8_t*) dram.getSpace(ssdBlockSize);
    }
    uint8_t** hddInputBuffers = new uint8_t*[numberOfHddRuns];
    for (size_t i = 0; i < numberOfHddRuns; i++) {
        hddInputBuffers[i] = (uint8_t*) dram.getSpace(hddBlockSize);
    }
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read input into DRAM-to-HDD input buffer
    size_t bytesRead = 0;
    while (inputOffset < bytesToFill) {
        if (inputHdd->readData(dramInputBuffer + bytesRead, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
    // Generate cache-size runs over the input buffer

    // Merge these runs with the SSD and HDD input buffers using Tree-of-Losers and output to HDD using hddOutputBuffer

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (size_t i = 0; i < numberOfHddRuns; i++) {
        (uint8_t*) dram.freeSpace(hddInputBuffers[i], hddBlockSize);
    }
    for (size_t i = 0; i < ssdMaxRunCount; i++) {
        (uint8_t*) dram.freeSpace(ssdInputBuffers[i], ssdBlockSize);
    }
    dram.freeSpace(dramInputBuffer, dramInputBufferSize);

    return inputOffset; 
}

int main(int argc, char* argv[]) {
    // Default values
    std::string o_filename = "o.txt";   // Output file name
    inputHdd = new Ssd("./input/testData.bin", totalDataSize, hddBlockSize);

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-c") == 0) {
            numberOfRecords = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-s") == 0) {
            rowSize = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-o") == 0) {
            o_filename = argv[i + 1];
        } 
    }
    
    totalDataSize = numberOfRecords * rowSize;  // Total amount of data in bytes
    outTrace.open(o_filename,  std::ios_base::out);
    TRACE(true);

    // Print the values
    cout << "Total number of records: " << numberOfRecords << endl;
    cout << "Size of one record: " << rowSize << endl;
    cout << "Output file name: " << o_filename << endl;

	//ScanIterator* const sc_it = new ScanIterator(new ScanPlan(totalDataSize, hddBlockSize));
    //sc_it->run();

    /**
     * Case 1: < 100 MB
     * Case 2: < 10 GB
     * Case 3: >= 10 GB
    */
    if (totalDataSize < dramLimit) { // Case 1: Unsorted HDD->DRAM->HDD Sorted
        sortDramAndStore(totalDataSize, 0, 0, FALSE, outputHdd);
    } else if (totalDataSize < ssdLimit) { // Case 2: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted
        sortSsdAndStore(totalDataSize, 0, 0, FALSE, outputHdd);
    } else { // Case 3: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted, Sorted DRAM + Sorted SSD + Sorted HDD->HDD Sorted
        sortHddAndStore(totalDataSize, 0);
    }
    return 0;
}  // main
