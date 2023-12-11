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

- **Quicksort**([Sort.cpp::_quickSort()_](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/Sort.cpp#L97)):
  We chose to implement quicksort for sorting smaller sized 500KB runs because its average time complexity is O(nlogn).
- **Tournament Trees**([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L526)):
  When using a tree of losers,
  all the records with which the winner has to be compared lie on a path from the winner leaf to the root. As long as each node in the tree has information about its parent, then it is very easy to find the next winner.
- **Run Size > Memory Size**([Main.cpp::sortHddAndStore()](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/Main.cpp#L261C1-L262C1)) For data greater than 10GB, we are taking runsizes of 9.9GB which is greater than the size of memory
- **Offset value coding** ([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L292))
  Helps in optimizing comparisons and improves time to run external merge sort
- **Minimum count of row & column comparisons** ([tol.cpp::Class TOL](https://github.com/natanlidukhover/CS764-Project/blob/151c72b0719a4629e72e2ad6d5970118f2cb0499/tol.cpp#L292))
  Implemented in TreeOfLosers and offset value coding to compare rows and find the first distinct column
- **Cache sized mini runs (tol.cpp::Class Run)**:
  Implemented using getNext and setNext methods
- Device optimized page sizes(Main.cpp::blockSize variable)
- **Verifying sets of rows and values/sort order(verification.cpp::verify())**
- takes two individual binary files and based off of designated row size and page size, verifies that all rows in the unsorted data are present in the sorted data, and that the sorted data is indeed sorted

- run size > memory size
- Spilling memory to SSD
- Spilling from SSD to Disk
- Graceful degradation
-

## Contributions

- Natan Lidukhover: Setting up buffers for HDD,SSD, DRAM and integrating TreeOfLosers in Main.cpp
- Shivam Hire: Implementing TreeOfLosers data structure, Run Class and Storage Class
- Smit Shah: File I/O for random numbers, generating unsorted data and SSD read write data methods
- Tyler Wilson: Random number generation and verification code for sorted and unsorted input
