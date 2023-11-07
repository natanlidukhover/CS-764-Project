#include "Filter.h"
#include "Iterator.h"
#include "Scan.h"
#include "Sort.h"
#include "Table.h"
#include "Dram.h"
#include "defs.h"
#include <iostream>

using namespace std;
int main(int argc, char* argv[]) {
    TRACE(true);

	int n = 10;// * 1024;
	Table tmp(n, n, 1);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			tmp[i][j] = Random(10);//n - i+j;
			cout << (int)tmp[i][j];
			cout << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int sort_col = 0;
	generateRuns(tmp, 0, n - 1, sort_col);
	verifySortedRuns(tmp, 0, n - 1, sort_col);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			cout << (int)tmp[i][j] << " ";
		}
		cout << "\n";
	}

	// Plan * const plan = new ScanPlan (7);
	// new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	// Iterator * const it = plan->init ();
	// it->run ();
	// delete it;

	// delete plan;

    return 0;
}  // main
