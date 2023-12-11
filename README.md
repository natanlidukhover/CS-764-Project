Deliverables:

Your submission should be a zip file that includes:

Your full source code with clear documentations

A makefile that compiles all your executables

And README file that includes:

    Each group member's names and student IDs

    The techniques (see Grading) implemented by your submission and the corresponding source files and lines

    A brief writeup of (1) the reasons you chose to implement the specific subset of techniques (2) the project's state (complete or have what kinds of bugs) (3) how to run your programs if they are different from the default specifications above. This is not expected to be a lengthy document, just to assist our understandings of your work.

    Each group member's contributions to the project

# Table of Contents

- [Group Members](#group-members)
- [Techniques implemented](#techniques-implemented)
- [Project State](#project-state)
- [Contributions](#contributions)

## Group Members:

Project Group 15 on Canvas. Following are the names along with the ID numbers

- Natan Lidukhover - 907 934 3324
- Shivam Hire - 908 272 5129
- Smit Shah - 908 587 6440
- Tyler Wilson - 908 622 6520

## Techniques Implemented:

- **Quicksort**([Sort.cpp::quickSort()](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/Sort.cpp#L97)):
  We chose to implement quicksort for sorting smaller sized 500KB runs because its average time complexity is O(n * log(n)).
- **Tournament trees**([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L526)):
  When using a tree of losers, all the records with which the winner has to be compared to lie on a path from the winner leaf to the root. It is very easy to find the next winner by calculating parent node indices and performing the necessary comp[arison].
- **Run size > memory size** ([Main.cpp::sortHddAndStore()](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L261C1-L262C1)):
  We do use run size greater than DRAM size for sorting large amounts of data to speed up sorting
- **Offset-value coding** ([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L292)):
  Helps in optimizing comparisons and improves time to run external merge sort
- **Minimum count of row & column comparisons** ([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L292)):
  Implemented in TOL (tree of losers) and offset-value coding to compare rows and find the first distinct column
- **Cache-size mini runs** ([Main.cpp::sortDramandStore](https://github.com/natanlidukhover/CS764-Project/blob/8870c03b8b417f5c89c76e2422b1c3402fd1def7/Main.cpp#L92)):
  We are generating cache-sized mini runs and then quick sorting them
- **Device-optimized page sizes**([Main.cpp::blockSize variable](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L34C1-L35C36)):
  Using device-optimized page sizes, we can use the entire bandwidth of the device rather than constraining it
- **Spilling memory to SSD**([Main.cpp::sortSsdandStore()](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L148C1-L148C1)):
  When the memory is full in DRAM, we utilize the SSD to store sorted runs and then merge them using TOL (tree of losers) and output the sorted output to SSD
- **Spilling from SSD to disk**([Main.cpp::sortHddAndStore()](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L350)):
  When the memory is full in the SSD merge step, we utilize the HDD to store sorted runs and then merge them using TOL (tree of losers) and store it on disk
- **Graceful degradation into merging**([Main.cpp::sortDramAndStore()](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L102)):
  Final merge step in DRAM which ensures graceful degradation
- **Graceful degradation into merging beyond one step**([Main.cpp::sortDramAndStore()](https://github.com/natanlidukhover/CS764-Project/blob/2da37b8da65a8621e9b1db6b9ed868b1148f2ff5/Main.cpp#L102)):
  Final merge step in HDD which internally uses SSD and DRAM merges to gracefully degrade
- **Verifying sort order** ([verification.cpp::verify()](https://github.com/natanlidukhover/CS764-Project/blob/8870c03b8b417f5c89c76e2422b1c3402fd1def7/verification.cpp#L26)):
  Helps to ensure that sort order is correct and that external merge sort is implemented
- **Verifying sets of rows and values**([verification.cpp::verify()](https://github.com/natanlidukhover/CS764-Project/blob/8870c03b8b417f5c89c76e2422b1c3402fd1def7/verification.cpp#L63)):
  Helps to ensure that the data is not corrupted and the input data matches with output data, and takes two individual binary files and (based off of designated row size and page size) verifies that all rows in the unsorted data are present in the sorted data, and that the sorted data is indeed sorted
- **Optimized merge patterns**([Main.cpp::sortHddandStore](https://github.com/natanlidukhover/CS764-Project/blob/8870c03b8b417f5c89c76e2422b1c3402fd1def7/Main.cpp#L262)):
  Implemented to make efficient use of all storage devices by pre-calculating the exact amount of DRAM needed plus the number of SSD and HDD runs that will be needed, and using DRAM->SSD->HDD as necessary. All the math calculating run sizes and buffer sizes implements the optimized merge patterns with three cases where either only DRAM, DRAM+SSD, or DRAM+SSD+HDD are needed.

## Usage and discussion
### Reasoning for techniques used
We used the offset-value coding and tree of losers technique in order to provide good efficiency and a minimal comparison count. The tree of losers uses an array-based structure and all node accesses are done by indexing and calculating indexes rather than traversing a linked list. We split up the main loop into three cases: DRAM sort only, DRAM + SSD sort, and DRAM + SSD + HDD sort in order to provide readable code while re-using code components for similar merging and sorting steps.

In the DRAM case, first the necessary output buffer size is set, and the remaining space (1-MB aligned) is used for the input buffer in order to maximize the amount that DRAM is used for quick access. Input is read in to DRAM from the specified input device. Cache-sized mini runs of 500KB are generated within DRAM and sorted using QuickSort due to its average case O(n * log(n)) time complexity and efficient O(log(n)) space complexity. These runs are then merged using tree of losers and output to the specified input device (case 1 is HDD).

In the SSD case, first the necessary output buffer size is set, and the remaining space (1-MB aligned) is pre-calculated for the input buffer to use in the next step. The number of 99MB (dramLimit - 1MB)-size runs is calculated, and the size of the last run (if it is less than the normal run size) is calculated. The above DRAM case is then run that many times but output to SSD instead of HDD. Then, the input buffer in DRAM that was previously pre-calculated is filled from the input device. Cache-sized mini runs of 500KB are generated within DRAM and sorted using QuickSort. These runs are then merged with the runs persisted on the SSD using tree of losers.

In the HDD case, first the necessary output buffer size is set, and the remaining space (1-MB aligned) is pre-calculated for the input buffer to use in the next step. The number of 9.9GB (ssdLimit)-size runs is calculated, and the size of the last run (if it is less than the normal run size) is calculated. The above SSD case is then run that many times but output to HDD. The above DRAM case is then run a maximal 100 times but output to SSD instead of HDD. Then, the input buffer in DRAM that was previously pre-calculated is filled from the input device. Cache-sized mini runs of 500KB are generated within DRAM and sorted using QuickSort. These runs are then merged with the runs persisted on the SSD and runs persisted on HDD using tree of losers. This all ensures that DRAM is used maximally before using SSD, then ensures that SSD is used maximally before using HDD.
### Project state
The project is complete, including a bonus credit for implementing optimized merge patterns. There are no bugs that we have seen in the final product. The program does not take into account a specific number of columns (each row is considered one column). This is not really a limitation as the sorting process is the exact same, and the debug prints the same way, so this has no difference in the final output.
### Running the program
The program can be run as follows (from the project base directory):
```bash
./Sort.exe -c [total number of records] -s [individual record size] -o [trace file]
```

To run the verification code rather than the sort process path, run it as follows. The sort process path must be run first in order to have data to verify at the necessary files (and the parameters must be identical):
```bash
./Sort.exe -c [total number of records] -s [individual record size] -o [trace file] -v
```

You can also optionally add a `-d` flag to run with debug mode turned on in order to get (first five bytes in each row) DRAM sorted run output to `output/dram.txt`, SSD sorted run output to `output/ssd.txt` (case 3), HDD sorted run output to `output/hdd.txt` (case 3), and final sorted output to `output/testData.txt`.
```bash
./Sort.exe -c [total number of records] -s [individual record size] -o [trace file] -d
```

Both `-d` and `-v` must be placed at the end of all the arguments, if present. Intermediate step sorted run output is available in the output folder in binary format as `ssd.bin` and `hdd.bin`.

## Contributions

- Natan Lidukhover: Setting up buffers for HDD, SSD, DRAM, and integrating TOL (tree of losers) in Main.cpp
- Shivam Hire: Implementing TOL (tree of losers) data structure, Run class, and Storage cClass
- Smit Shah: File I/O for random numbers, generating unsorted data, and SSD read/write data methods
- Tyler Wilson: Random number generation and verification code for sorted and unsorted input
