#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>
#include "Ssd.h"
#include <fstream>
#define cout outTrace

using namespace std;
int main(int argc, char* argv[]) {

    // Default values
   	size_t number_of_records = 24;//50MB=50 * 1024 * 1024; //in bytes
	size_t row_size = 6;
	size_t blockSize = 3;
    std::string o_filename="o.txt";

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
	//for hdd blockSize is given by bandwidth * latency = 100*0.01
	Ssd *  unsorted_hdd = new Ssd("./input/testData.bin",(size_t)number_of_records*row_size, blockSize);
    Ssd *  sorted_hdd = new Ssd("./output/testData.bin",(size_t)number_of_records*row_size, blockSize);	
	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (number_of_records));
	vector<int> numbers = sc_it->run();
	
	uint8_t *data = (uint8_t *)dram.getSpace(1 * pow(10, 6));
    //Run initRun(unsorted_hdd, data, (size_t)(1 * 1e6), (size_t)0, number_of_records * row_size, number_of_records * row_size, (size_t)1e6, row_size, (size_t)0);
	for (size_t i = 0; i < number_of_records * row_size; i += blockSize) {
		int bytesread = unsorted_hdd->readData(data + i, i);
		cout << bytesread << endl;
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

	quickSort(data, number_of_records, row_size);
	verifySortedRuns(data, number_of_records, row_size);

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
		sorted_hdd->writeData(static_cast<const void*>(data + (i * blockSize)),offset + i* blockSize);
	}
    return 0;
}  // main
