#include <math.h>

#include <fstream>
#include <iostream>

#include "Dram.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Ssd.h"
#include "defs.h"
#include "tol.h"
#include "verification.h"

#define cout outTrace
#define KB (size_t) 1000
#define MB 1000 * KB
#define GB 1000 * MB
#define FALSE 0
#define TRUE 1

using namespace std;
std::ofstream outTrace;
std::ofstream dramDebugOut;
std::ofstream ssdDebugOut;
std::ofstream hddDebugOut;
std::ofstream finalDebugOut;

int debug = 0;
int dramDebug = 0;
int ssdDebug = 0;

const size_t ssdBlockSize = 10 * KB;  // For SSD, block size is given by bandwidth * latency = 100 MB/s * 0.0001 s = 10 KB
const size_t hddBlockSize = 1 * MB;   // For HDD, block size is given by bandwidth * latency = 100 MB/s * 0.01 s = 1 MB
const size_t cacheLimit = 1 * MB;
const size_t dramLimit = 100 * MB;
const size_t ssdLimit = 10 * GB;
const size_t hddLimit = 900 * GB;
const size_t ssdMaxRunCount = 100;
const size_t hddMaxRunCount = 97;
const size_t ssdMaxInputBufferSize = ssdMaxRunCount * ssdBlockSize;
const size_t hddMaxInputBufferSize = hddMaxRunCount * hddBlockSize;
const size_t cacheRunSize = 500 * KB;
size_t numberOfRecords = 1 * MB;  // Number of rows
size_t rowSize = 50;           // Size of each row in bytes
size_t totalDataSize = numberOfRecords * rowSize;
Ssd* inputHdd;
Ssd* interimHdd;
Ssd* outputSsd;
Ssd* outputHdd;

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 * @param const size_t outputStartOffset: the start byte offset of the output storage device
 * @param const int isOutputSsd: true if output storage device is an SSD, false otherwise
 * @param Ssd* outputDevice: the pointer to the Ssd object of the output storage device
 *
 * @return size_t: the offset of the input storage device of next block not read yet
 *
 * The function reads data from the input storage device into a DRAM buffer,
 * generates cache-size runs over this input, sorts these cache-size runs, merges
 * the runs together, and pushes the sorted output to the given storage device through an output buffer
 */
size_t sortDramAndStore(const size_t bytesToFill, const size_t inputStartOffset, const size_t outputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    TRACE(true);
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    const size_t numberOfCacheRuns = (bytesToFill / cacheRunSize) + (bytesToFill % cacheRunSize != 0);  // Round up
    const size_t outputBufferSize = outputBlockSize;
    const size_t inputBufferSize = dramLimit - outputBufferSize;

    // Create buffers
    uint8_t* inputBuffer = (uint8_t*) dram.getSpace(inputBufferSize);
    uint8_t* outputBuffer = (uint8_t*) dram.getSpace(outputBufferSize);

    // Read into DRAM input buffer from the given input storage device starting at inputStartOffset
    size_t inputOffset = inputStartOffset;
    size_t bytesRead = 0;
    while (inputOffset < (inputStartOffset + bytesToFill)) {
        if (inputHdd->readData(inputBuffer + bytesRead, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
    cout << "Read " << bytesRead << " bytes from HDD to DRAM and using quicksort to generate " << numberOfCacheRuns << " runs each of size " << cacheRunSize << endl;

    // Generate cache-size runs over the input buffer and sort them
    Run** runs = new Run*[numberOfCacheRuns];
    for (size_t i = 0; i < numberOfCacheRuns; i++) {
        runs[i] = new Run(inputHdd, inputBuffer + (i * cacheRunSize), inputBlockSize, 0, 0, cacheRunSize, cacheRunSize, rowSize, cacheRunSize);
        quickSort(inputBuffer + (i * cacheRunSize), cacheRunSize / rowSize, rowSize);
    }
    cout << "All runs needed for merging using DRAM generated" << endl;
    Run* outputRun = new Run(outputDevice, outputBuffer, outputBlockSize, outputStartOffset, 0, bytesToFill, outputBufferSize, rowSize, 0);

    // Merge these runs using Tree-of-Losers and output to the given output storage device
    ETable* t = new ETable(numberOfRecords, rowSize, rowSize, 0);

    cout << "Using tree of losers to merge " << numberOfCacheRuns << " runs" << endl;
    TOL* tol = new TOL(numberOfCacheRuns, runs, outputRun, *t);
    size_t numberOfTolRecords = bytesToFill / rowSize;
    for (size_t i = 0; i < numberOfTolRecords + 1; i++) {
        tol->pass();
    }
    cout << "TOL outputted " << numberOfTolRecords << " records" << endl;
    outputRun->flush();
    if (debug && ssdDebug) {
        for (size_t i = 0; i < numberOfTolRecords; i++) {
            uint8_t* ptr;
            outputRun->getNext(&ptr);
            ssdDebugOut << "[" << i + 1 << "]";
            for (size_t j = 0; j < 5; j++) {
                ssdDebugOut << (int) ptr[j] << " ";
            }
            ssdDebugOut << endl;
        }
    }
    if (debug && dramDebug) {
        for (size_t i = 0; i < numberOfTolRecords; i++) {
            uint8_t* ptr;
            outputRun->getNext(&ptr);
            dramDebugOut << "[" << i + 1 << "]";
            for (size_t j = 0; j < 5; j++) {
                dramDebugOut << (int) ptr[j] << " ";
            }
            dramDebugOut << endl;
        }
    }

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    dram.freeSpace(inputBuffer, inputBufferSize);

    return inputOffset;
}

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 * @param const size_t outputStartOffset: the start byte offset of the output storage device
 * @param const int isOutputSsd: true if output storage device is an SSD, false otherwise
 * @param Ssd* outputDevice: the pointer to the Ssd object of tthe output storage device
 *
 * @return size_t: the offset of the input storage device of the next block not read yet
 *
 * The function reads data from the input storage device, optimally uses DRAM to sort runs of these
 * bytes and stores the sorted runs on SSD (using sortDramAndStore()),
 * then merges the sorted SSD runs with sorted cache-sized runs in DRAM to the specified output storage device
 */
size_t sortSsdAndStore(const size_t bytesToFill, const size_t inputStartOffset, const size_t outputStartOffset, const int isOutputSsd, Ssd* outputDevice) {
    TRACE(true);
    // Create constants
    size_t outputBlockSize = hddBlockSize;
    if (isOutputSsd) {
        outputBlockSize = ssdBlockSize;
    }
    const size_t inputBlockSize = hddBlockSize;
    const size_t outputBufferSize = outputBlockSize;
    const size_t ssdRunSize = dramLimit - outputBufferSize;
    const size_t minDramInputBufferSize = dramLimit - hddBlockSize - ssdMaxInputBufferSize;
    const size_t ssdMaxDataSize = bytesToFill - minDramInputBufferSize;
    const size_t numberOfSsdRuns = (ssdMaxDataSize / ssdRunSize) + (ssdMaxDataSize % ssdRunSize != 0);  // Round up
    size_t dramInputBufferSize = dramLimit - outputBufferSize - (numberOfSsdRuns * ssdBlockSize);
    dramInputBufferSize = (dramInputBufferSize % inputBlockSize == 0) ? dramInputBufferSize : (dramInputBufferSize + inputBlockSize - (dramInputBufferSize % inputBlockSize));
    const size_t ssdActualDataSize = bytesToFill - dramInputBufferSize;
    const size_t numberOfCacheRuns = (dramInputBufferSize / cacheRunSize) + (dramInputBufferSize % cacheRunSize != 0);  // Round up
    const size_t ssdLastRunSize = (ssdActualDataSize % ssdRunSize == 0) ? ssdRunSize : (ssdActualDataSize % ssdRunSize);

    // Perform input-to-SSD sort and merge through DRAM
    size_t inputOffset = inputStartOffset;
    size_t ssdBytesLeft = ssdActualDataSize;
	size_t tNumSSDSizeRunGenerated = ssdBytesLeft / ssdRunSize + (ssdBytesLeft % ssdRunSize != 0);
	cout << "Generating " << tNumSSDSizeRunGenerated << " run each of size " << ssdRunSize << " or " << ssdLastRunSize << " using DRAM and storing it in SSD" << endl;
    while (ssdBytesLeft > 0) {
        size_t minRunSize = std::min(ssdRunSize, ssdBytesLeft);
        inputOffset = sortDramAndStore(minRunSize, inputOffset, ssdActualDataSize - ssdBytesLeft, TRUE, outputSsd);
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
    while (inputOffset < (inputStartOffset + bytesToFill)) {
        if (inputHdd->readData(dramInputBuffer + bytesRead, inputOffset) <= 0) {
            break;
        }
        bytesRead += inputBlockSize;
        inputOffset += inputBlockSize;
    }
	cout << "Generating " << numberOfCacheRuns << " runs each of size " << cacheRunSize << " and storing it in DRAM" << endl;
    // Generate cache-size runs over the input buffer and sort them
    Run** runs = new Run*[numberOfCacheRuns + numberOfSsdRuns];
    for (size_t i = 0; i < numberOfCacheRuns; i++) {
        runs[i] = new Run(inputHdd, dramInputBuffer + (i * cacheRunSize), inputBlockSize, 0, 0, cacheRunSize, cacheRunSize, rowSize, cacheRunSize);
        quickSort(dramInputBuffer + (i * cacheRunSize), cacheRunSize / rowSize, rowSize);
    }
	cout << "All runs needed for merging using DRAM and SSD generated" << endl;
    for (size_t i = 0; i < numberOfSsdRuns; i++) {
        size_t minRunSize = ssdRunSize;
        if (i == numberOfSsdRuns - 1) {
            minRunSize = ssdLastRunSize;
        }
        runs[numberOfCacheRuns + i] = new Run(outputSsd, ssdInputBuffers[i], ssdBlockSize, i * ssdRunSize, minRunSize, minRunSize, ssdBlockSize, rowSize, 0);
    }
	cout << numberOfSsdRuns << " buffers allocated for runs in SSD and " << numberOfCacheRuns << " buffers allocated for runs in DRAM" << endl;
    Run* outputRun = new Run(outputDevice, outputBuffer, outputBlockSize, outputStartOffset, 0, bytesToFill, outputBufferSize, rowSize, 0);

    // Merge these runs with the SSD input buffers using Tree-of-Losers and output to HDD using outputBuffer
    ETable* t = new ETable(numberOfRecords, rowSize, rowSize, 0);

	cout << "Using Tree-of-loosers to merge " << numberOfCacheRuns + numberOfSsdRuns << " runs" << endl;
    TOL* tol = new TOL(numberOfCacheRuns + numberOfSsdRuns, runs, outputRun, *t);
    const size_t numberOfCacheRecords = dramInputBufferSize / rowSize;
    const size_t numberOfSsdRecords = ssdActualDataSize / rowSize;
    const size_t numberOfTolRecords = numberOfCacheRecords + numberOfSsdRecords;
    for (size_t i = 0; i < numberOfTolRecords + 1; i++) {
        tol->pass();
    }
    cout << "TOL outputted " << numberOfRecords << " records" << endl;
    outputRun->flush();
    if (debug) {
        for (size_t i = 0; i < numberOfTolRecords; i++) {
            uint8_t* ptr;
            outputRun->getNext(&ptr);
            hddDebugOut << "[" << i + 1 << "]";
            for (size_t j = 0; j < 5; j++) {
                hddDebugOut << (int) ptr[j] << " ";
            }
            hddDebugOut << endl;
        }
    }

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (int i = (int) numberOfSsdRuns - 1; i >= 0; i--) {
        dram.freeSpace(ssdInputBuffers[i], ssdBlockSize);
    }
    dram.freeSpace(dramInputBuffer, dramInputBufferSize);

    return inputOffset;
}

/**
 * @param const size_t bytesToFill: how many bytes to read into DRAM from the input storage device
 * @param const size_t inputStartOffset: the start byte offset of the input storage device
 *
 * @return size_t: the offset of the input storage device of the next block not read yet
 *
 * The function reads data from the input storage device, optimally uses DRAM + SSD to sort runs of these
 * bytes and stores the sorted runs on HDD (using sortSsdAndStore()), then optimally uses DRAM to sort runs and store
 * the sorted runs on SSD (using sortDramAndStorage), then merges the sorted SSD and HDD runs with sorted cache-sized
 * runs in DRAM to the specified output storage device
 */
size_t sortHddAndStore(const size_t bytesToFill, const size_t inputStartOffset) {
    TRACE(true);
    // Create constants
    const size_t outputBlockSize = hddBlockSize;
    const size_t inputBlockSize = hddBlockSize;
    const size_t outputBufferSize = outputBlockSize;
    const size_t ssdRunSize = dramLimit - outputBlockSize;
    const size_t ssdActualDataSize = ssdMaxRunCount * ssdRunSize;
    const size_t hddRunSize = ssdActualDataSize;
    const size_t minDramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize - hddMaxInputBufferSize;
    const size_t hddMaxDataSize = bytesToFill - ssdActualDataSize - minDramInputBufferSize;
    const size_t numberOfHddRuns = (hddMaxDataSize / hddRunSize) + (hddMaxDataSize % hddRunSize != 0);  // Round up
    size_t dramInputBufferSize = dramLimit - outputBufferSize - ssdMaxInputBufferSize - (numberOfHddRuns * hddBlockSize);
    dramInputBufferSize = (dramInputBufferSize % inputBlockSize == 0) ? dramInputBufferSize : (dramInputBufferSize + inputBlockSize - (dramInputBufferSize % inputBlockSize));
    const size_t hddActualDataSize = bytesToFill - ssdActualDataSize - dramInputBufferSize;
    const size_t numberOfCacheRuns = (dramInputBufferSize / cacheRunSize) + (dramInputBufferSize % cacheRunSize != 0);  // Round up
    const size_t ssdLastRunSize = (ssdActualDataSize % ssdRunSize == 0) ? ssdRunSize : (ssdActualDataSize % ssdRunSize);
    const size_t hddLastRunSize = (hddActualDataSize % hddRunSize == 0) ? hddRunSize : (hddActualDataSize % hddRunSize);

    // Perform input-to-SSD-to-HDD sort
    size_t inputOffset = inputStartOffset;
    size_t hddBytesLeft = hddActualDataSize;
	size_t tNumSSDSizeRunGenerated = hddBytesLeft / hddRunSize + (hddBytesLeft % hddRunSize != 0);
	cout << "Generating " << tNumSSDSizeRunGenerated << " run each of size " << hddRunSize << " or " << hddLastRunSize << " using DRAM and SSD and storing it in HDD" << endl;
    while (hddBytesLeft > 0) {
        size_t minRunSize = std::min(hddRunSize, hddBytesLeft);
        inputOffset = sortSsdAndStore(minRunSize, inputOffset, hddActualDataSize - hddBytesLeft, FALSE, interimHdd);
        hddBytesLeft -= minRunSize;
    }

    if (debug) {
        ssdDebug = 1;
    }
    // Perform input-to-SSD sort
    size_t ssdBytesLeft = ssdActualDataSize;
	size_t tNumDRAMSizeRunGenerated = ssdBytesLeft / ssdRunSize + (ssdBytesLeft % ssdRunSize != 0);
	cout << "Generating " << tNumDRAMSizeRunGenerated << " run using DRAM and storing it in SSD" << endl;
    while (ssdBytesLeft > 0) {
        size_t minRunSize = std::min(ssdRunSize, ssdBytesLeft);
        inputOffset = sortDramAndStore(minRunSize, inputOffset, ssdActualDataSize - ssdBytesLeft, TRUE, outputSsd);
        ssdBytesLeft -= minRunSize;
    }
    if (debug) {
        ssdDebug = 0;
    }

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
	cout << "Read " << bytesRead << " from HDD to DRAM and using quicksort to generate " << numberOfCacheRuns << " runs each of size " << cacheRunSize << endl;
    // Generate cache-size sorted runs over the input buffer
    Run** runs = new Run*[numberOfCacheRuns + ssdMaxRunCount + numberOfHddRuns];
    for (size_t i = 0; i < numberOfCacheRuns; i++) {
        runs[i] = new Run(inputHdd, dramInputBuffer + (i * cacheRunSize), inputBlockSize, 0, 0, cacheRunSize, cacheRunSize, rowSize, cacheRunSize);
        quickSort(dramInputBuffer + (i * cacheRunSize), cacheRunSize / rowSize, rowSize);
    }
    if (debug) {
        for (size_t i = 0; i < dramInputBufferSize / rowSize; i++) {
            uint8_t* ptr;
            ptr = dramInputBuffer + (i * rowSize);
            dramDebugOut << "[" << i + 1 << "]";
            for (size_t j = 0; j < 5; j++) {
                dramDebugOut << (int) ptr[j] << " ";
            }
            dramDebugOut << endl;
        }
    }
	cout << "All runs needed for final merging generated" << endl;
    for (size_t i = 0; i < ssdMaxRunCount; i++) {
        size_t minRunSize = ssdRunSize;
        if (i == ssdMaxRunCount - 1) {
            minRunSize = ssdLastRunSize;
        }
        runs[numberOfCacheRuns + i] = new Run(outputSsd, ssdInputBuffers[i], ssdBlockSize, i * ssdRunSize, minRunSize, minRunSize, ssdBlockSize, rowSize, 0);
    }
    cout << ssdMaxRunCount << " buffers allocated for runs in SSD" << endl;
    for (size_t i = 0; i < numberOfHddRuns; i++) {
        size_t minRunSize = hddRunSize;
        if (i == numberOfHddRuns - 1) {
            minRunSize = hddLastRunSize;
        }
        runs[numberOfCacheRuns + ssdMaxRunCount + i] = new Run(interimHdd, hddInputBuffers[i], hddBlockSize, i * hddRunSize, minRunSize, minRunSize, hddBlockSize, rowSize, 0);
    }
	cout << numberOfHddRuns << " buffers allocated for runs in HDD" << endl;
    Run* outputRun = new Run(outputHdd, outputBuffer, outputBlockSize, 0, 0, bytesToFill, outputBufferSize, rowSize, 0);

    // Merge these runs with the SSD and HDD input buffers using Tree-of-Losers and output to HDD using hddOutputBuffer
    ETable* t = new ETable(numberOfRecords, rowSize, rowSize, 0);

	cout << "Using Tree-of-loosers to merge " << numberOfCacheRuns + ssdMaxRunCount + numberOfHddRuns << " runs" << endl;
    TOL* tol = new TOL(numberOfCacheRuns + ssdMaxRunCount + numberOfHddRuns, runs, outputRun, *t);
    for (size_t i = 0; i < numberOfRecords + 1; i++) {
        tol->pass();
    }
    cout << "TOL outputted " << numberOfRecords << " records" << endl;
    outputRun->flush();
    if (debug) {
        for (size_t i = 0; i < numberOfRecords; i++) {
            uint8_t* ptr;
            outputRun->getNext(&ptr);
            finalDebugOut << "[" << i + 1 << "]";
            for (size_t j = 0; j < 5; j++) {
                finalDebugOut << (int) ptr[j] << " ";
            }
            finalDebugOut << endl;
        }
    }

    // Free buffers
    dram.freeSpace(outputBuffer, outputBufferSize);
    for (int i = (int) numberOfHddRuns - 1; i >= 0; i--) {
        dram.freeSpace(hddInputBuffers[i], hddBlockSize);
    }
    for (int i = (int) ssdMaxRunCount - 1; i >= 0; i--) {
        dram.freeSpace(ssdInputBuffers[i], ssdBlockSize);
    }
    dram.freeSpace(dramInputBuffer, dramInputBufferSize);

    return inputOffset;
}

int main(int argc, char* argv[]) {
    // Default values
    std::string outputFilename = "trace.txt";  // Output file name
    int runVerification = 0;

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-c") == 0) {
            numberOfRecords = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-s") == 0) {
            rowSize = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFilename = argv[i + 1];
        } else if (strcmp(argv[i], "-v") == 0) {
            runVerification = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
            debug = 1;
        }
    }

	if (argc != 7 && debug != 1 && runVerification != 1) {
		std::cerr << "Usage: ./sort -c [total number of records] -s [individual record size] -o [trace file]" << endl;
		return EINPARM;
	}
	if ((numberOfRecords * rowSize) % ((size_t) 1 * MB) != 0) {
		std::cerr << "((Number of records) * (record size)) should be 1 MB-aligned" << endl;
		return EINPARM;
	}

    outTrace.open(outputFilename, std::ios_base::out);
    TRACE(true);

    if (debug) {
        dramDebugOut.open("./output/dram.txt", std::ios_base::out);
        ssdDebugOut.open("./output/ssd.txt", std::ios_base::out);
        hddDebugOut.open("./output/hdd.txt", std::ios_base::out);
        finalDebugOut.open("./output/testData.txt", std::ios_base::out);
    }

    if (runVerification) {
        // Verification
        cout << "Running verification check" << endl;
        const char* outputFileName = "./output/testData.bin";
        const char* inputFileName = "./input/testData.bin";
        verify(outputFileName, inputFileName, hddBlockSize, rowSize, numberOfRecords);
    } else {
        inputHdd = new Ssd("./input/testData.bin", totalDataSize, hddBlockSize);
        interimHdd = new Ssd("./output/hdd.bin", totalDataSize, hddBlockSize);
        outputSsd = new Ssd("./output/ssd.bin", ssdLimit, ssdBlockSize);
        outputHdd = new Ssd("./output/testData.bin", totalDataSize, hddBlockSize);
        totalDataSize = numberOfRecords * rowSize;  // Total amount of data in bytes

        // Print the values
        cout << "-------------------------------Input--------------------------" << endl;
        cout << "Total number of records: " << numberOfRecords << endl;
        cout << "Size of one record: " << rowSize << endl;
        cout << "Trace file name: " << outputFilename << endl;
        cout << "Total data to be sorted in bytes: " << numberOfRecords * rowSize << endl
             << endl;

        cout << "----------------------Random number generator-----------------" << endl;
        ScanIterator* const sc_it = new ScanIterator(new ScanPlan(totalDataSize, hddBlockSize));
        sc_it->run();
        cout << endl;

        /**
         * Case 1: < 100 MB
         * Case 2: < 10 GB
         * Case 3: >= 10 GB
         */
        cout << "----------------------Starting sort------------" << endl;
        if (totalDataSize < dramLimit) {  // Case 1: Unsorted HDD->DRAM->HDD Sorted
            cout << "Case 1: Data less than DRAM size. Only using DRAM to sort." << std::endl;
            if (debug) {
                dramDebug = 1;
            }
            sortDramAndStore(totalDataSize, 0, 0, FALSE, outputHdd);
            if (debug) {
                dramDebug = 0;
            }
        } else if (totalDataSize < ssdLimit) {  // Case 2: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted
			cout << "Case 2: Data less than DRAM + SSD size. Only using DRAM and SSD to sort." << std::endl;
            sortSsdAndStore(totalDataSize, 0, 0, FALSE, outputHdd);
        } else {  // Case 3: Unsorted HDD->DRAM->SSD Sorted, Sorted DRAM + Sorted SSD->HDD Sorted, Sorted DRAM + Sorted SSD + Sorted HDD->HDD Sorted
            cout << "Case 3: Data less than DRAM + SSD + HDD size. Using DRAM, SSD and HDD to sort." << std::endl;
            sortHddAndStore(totalDataSize, 0);
        }
		cout << "Sorting of " << numberOfRecords << " records, each of size " << rowSize << " bytes completed successfully!!!" << endl;
		cout << endl;


		cout << "----------------------Statistics about sorting------------" << endl;
		size_t numHddBlocksWritten = inputHdd->getWriteCount() + outputHdd->getWriteCount() + interimHdd->getWriteCount();
		size_t numHddBlocksRead = inputHdd->getReadCount() + outputHdd->getReadCount() + interimHdd->getReadCount();
		cout << "HDD block size is: " << hddBlockSize << endl;
		cout << "Number of blocks written to HDD: " << numHddBlocksWritten << endl;
		cout << "Number of blocks read from HDD: " << numHddBlocksRead << endl;
		cout << "Number of blocks read from HDD: " << inputHdd->getReadCount() << endl;
		cout << "Number of blocks read from HDD: " << outputHdd->getReadCount() << endl;
		cout << "Number of blocks read from HDD: " << interimHdd->getReadCount() << endl;
		cout << "Total HDD access latency: " << (numHddBlocksWritten + numHddBlocksRead) * 10 << " ms" << endl;
		cout << endl;

		size_t numSsdBlocksWritten = outputSsd->getWriteCount();
		size_t numSsdBlocksRead = outputSsd->getReadCount();
		cout << "SSD block size is: " << ssdBlockSize << endl;
		cout << "Number of blocks written to SSD: " << numSsdBlocksWritten << endl;
		cout << "Number of blocks read from SSD: " << numSsdBlocksRead << endl;
		cout << "Total SSD access latency: " << (numSsdBlocksWritten + numSsdBlocksRead) * 0.1 << " ms" << endl;
    }

    return 0;
}  // main
