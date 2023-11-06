#include <stdint.h>

#include "Dram.h"

Dram::Dram(size_t Size): _Size(Size), _SizeOccupied(0)
{
	FreePtr = StartPtr = malloc(Size);
}

Dram::~Dram()
{
	free(StartPtr);
}

void *Dram::GetSpace(size_t Size)
{
	if(_SizeOccupied + Size > _Size)
		return NULL;
		// TODO Add exception raising code
	void *ptr = FreePtr;
	FreePtr = (uint8_t *)FreePtr + Size;
	_SizeOccupied += Size;
	return ptr;
}

void *Dram::FreeSpace(void *ptr, size_t size)
{
	uint8_t *byte_ptr = (uint8_t *)ptr;
	if (byte_ptr + size == FreePtr)
	{
		FreePtr = (uint8_t *)FreePtr - size;
		_SizeOccupied -= size;
		return byte_ptr;
	}
	else
		return NULL;
}

Dram dram(100 * 1024 * 1024);
