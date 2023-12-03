#include "defs.h"
#include "Ssd.h"
#include "Tol.h"

#include <cmath>
#include <math.h>
#include <stdexcept>


Storage::Storage(Ssd* storage, uint8_t* _d, size_t bs, size_t ss) : blockSize(bs), srcSeek(ss), d(_d), s(storage) {
}

Storage::~Storage() {
}

Run::Run(Ssd* _s, uint8_t* _d, size_t bs, size_t ss, size_t rs) : runSize(rs) {
    head = 0;
    tail = 0;
    source = new Storage(_s, _d, bs, ss);
}

Run::~Run() {
    delete source;
}

int Run::getNext(size_t s, size_t offset, size_t srcSeek) {
    // think about how will data move from all levels and in the data structures
    if (s % source->blockSize) {
        return EINPARM;
    }

    if (head >= tail) {
        return EEMPTY;
    }

    size_t data_read = 0;
	int ret = SUCCESS;
    for (size_t i = head; i < tail && i < ((size_t) head + s); i += source->blockSize) {
        if (ret == source->s->readData(source->d + data_read, srcSeek + i)) {
            return ret;
        }
        data_read += source->blockSize;
    }

    head += data_read;
    return SUCCESS;
}

int Run::setNext(size_t s, size_t offset, size_t srcSeek) {
    if (s % source->blockSize) {
        return EINPARM;
	}
    if (tail >= runSize) {
        return EFULL;
	}

    size_t data_written = 0;
	int ret = SUCCESS;
    for (size_t i = tail; i < runSize && i < ((size_t) tail + s); i += source->blockSize) {
        if (ret == source->s->writeData(source->d + data_written, srcSeek + i)) {
            return ret;
        }
        data_written += source->blockSize;
    }

    tail += data_written;
    return SUCCESS;
}

uint8_t* Run::getBuf() {
    return source->d;
}

size_t Run::getSize() {
    return runSize;
}

TOL::TOL(size_t nor, Run** rl, Run* o) : runList(rl), output(o), numOfRun(nor) {
    if (nor > 256) {
        throw std::runtime_error("TOL doesn't fit in cache");
	}
    tol_height = ceil(log2(nor)) + 1;
}
