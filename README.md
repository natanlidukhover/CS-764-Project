# CS764-Project

## Steps to Run

Compile the project using make command
Test.cpp is the entry point, hence run ./Test.exe after compilation

## Contributors

- Natan
- Shivam Hire
- Smit Shah
- Tyler Wilson

## Method Documentation

Ssd->writeData(const void \* data, size_t offset)
Write data at that particular offset. The caller will decide what data needs to be passed
and the offset where it needs to be written. It will write _blockSize amount of data. 
