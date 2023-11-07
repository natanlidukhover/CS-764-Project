#ifndef SSD_H
#define SSD_H

#include <cstddef>
#include <new> // Required for std::bad_alloc

class Ssd {
private:
    size_t _size;          // Total size of the SSD
    size_t _sizeOccupied;  // Currently occupied size
    void* startPtr;        // Starting pointer of the SSD memory block

public:
    // Constructor to initialize an SSD with a given size
    Ssd(size_t size);

    // Destructor to free allocated memory
    ~Ssd();

    // Function to simulate writing data to the SSD
    void* writeData(const void* data, size_t size);

    // Function to simulate reading data from the SSD
    void* readData(void* buffer, size_t size, size_t offset);

};

#endif // SSD_H
