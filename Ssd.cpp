#include "Ssd.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

Ssd::Ssd(size_t size) : _size(size), _sizeOccupied(0) {
    startPtr = malloc(size);
    if (startPtr == nullptr) {
        throw std::bad_alloc();
    }
    // In a real SSD, here would be the initialization of various SSD structures and metadata.
}

Ssd::~Ssd() {
    free(startPtr);
    // In a real SSD, here would be the clean-up of SSD structures and metadata.
}

void* Ssd::writeData(const void* data, size_t size) {
    if (_sizeOccupied + size > _size) {
        return nullptr;
    }
    void* writePtr = static_cast<uint8_t*>(startPtr) + _sizeOccupied;
    memcpy(writePtr, data, size);
    _sizeOccupied += size;

    // Simulate slower write times and other SSD characteristics here
    return writePtr;
}

void* Ssd::readData(void* buffer, size_t size, size_t offset) {
    if (offset + size > _sizeOccupied) {
        return nullptr;
    }
    void* readPtr = static_cast<uint8_t*>(startPtr) + offset;
    memcpy(buffer, readPtr, size);
    return buffer;
}

// In a real SSD, you would also need to handle delete operations, wear leveling, and other SSD-specific tasks.


int main() {
    try {
        // Create a simulated SSD with 100MB of storage
        Ssd ssd(100 * 1024 * 1024);

        // Simulate writing data to the SSD
        const char* data = "This is a test string to write to SSD";
        void* writePtr = ssd.writeData(data, strlen(data) + 1);  // +1 for the null-terminator

        if (writePtr != nullptr) {
            std::cout << "Data written to SSD at address: " << writePtr << std::endl;
        } else {
            std::cout << "Failed to write data to SSD." << std::endl;
        }

        // Simulate reading the same data from the SSD
        char buffer[1024];  // Buffer to read data into
        void* readPtr = ssd.readData(buffer, strlen(data) + 1, 0);  // Read from the start

        if (readPtr != nullptr) {
            std::cout << "Data read from SSD: " << buffer << std::endl;
        } else {
            std::cout << "Failed to read data from SSD." << std::endl;
        }

    } catch (const std::bad_alloc& e) {
        std::cerr << "Failed to allocate SSD memory: " << e.what() << std::endl;
    }

    return 0;
}
