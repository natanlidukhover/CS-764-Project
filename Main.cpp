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

using namespace std;
int main_buggy(int argc, char* argv[]) {

    // Default values
   	size_t number_of_records = 5 ; //in bytes
	size_t row_size = 1; // size of 1 row: 50bytes
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
    std::ofstream  outTrace;
	outTrace.open(o_filename,  std::ios_base::out );
    TRACE(true, outTrace);

 	// Print the values
    cout << "Total number of records " << number_of_records << std::endl;
    cout << "Size of one record" << row_size << std::endl;
    cout << "Output_filename: " << o_filename << std::endl;
    Ssd *  hdd = new Ssd("./input/testData.bin",(size_t)number_of_records*row_size, pow(10,6), number_of_records*row_size, outTrace);

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
	//for hdd pagesize is given by bandwidth * latency = 100*0.01
	
	size_t offset = (number_of_records*row_size) + 1;
	for(size_t i = 0; i < number_of_records; i++)
	{
		hdd->writeData(static_cast<const void*>(tmp[i]),offset + i*(row_size));
	}
	//hdd->writeData(tmp, offset);
    return 0;
}  // main
