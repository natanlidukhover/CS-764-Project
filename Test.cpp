#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>
#include "Ssd.h"
using namespace std;
int main(int argc, char* argv[]) {
    TRACE(true);

    // Default values
   	size_t data_size = 150;//50MB=50 * 1024 * 1024; //in bytes
	size_t row_size = 50;
    std::string o_filename;

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-c") == 0) {
            data_size = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-s") == 0) {
            row_size = std::stoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-o") == 0) {
            o_filename = argv[i + 1];
        } 
		// else {
        //     std::cerr << "Invalid option: " << argv[i] << std::endl;
        //     return 1; // Return an error code
        // }
    }

    // Print the values
    std::cout << "Total number of records " << data_size << std::endl;
    std::cout << "Size of one record" << row_size << std::endl;
    std::cout << "Output_filename: " << o_filename << std::endl;


	size_t rows = (data_size/row_size) + ((data_size % row_size) == 0 ? 0 : 1) ;
	ScanIterator * const sc_it = new ScanIterator(new ScanPlan (data_size));
	vector<int> numbers = sc_it->run();
	
	Table tmp(rows, 3, row_size/3);
	size_t k = 0;
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < row_size; j++) {
			tmp[i][j] = numbers[k++];
			cout << (int)tmp[i][j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int sort_col = 0;
	generateRuns(tmp, 0, rows - 1, sort_col);
	verifySortedRuns(tmp, 0, rows - 1, sort_col);

	cout << "Sorted table" << "\n";
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < row_size; j++) {
			cout << (int)tmp[i][j] << " ";
		}
		cout << "\n";
	}
	//for hdd pagesize is given by bandwidth * latency = 100*0.01
	Ssd * const hdd = new Ssd("./input/testData.bin",(size_t)10*1024*1024*1024, 1, data_size);
	size_t offset = data_size + 1;
	for(size_t i = 0; i < rows; i++)
	{
		hdd->writeData(static_cast<const void*>(tmp[i]),offset + i*(row_size),row_size);
	}

    return 0;
}  // main
