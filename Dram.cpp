#include "Dram.h"
#include <stdlib.h>

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
	FreePtr += Size;
	_SizeOccupied += Size;
	return ptr;
}

int Dram::FreeSpace(void *ptr, size_t size)
{
	if (ptr + size == FreePtr)
	{
		FreePtr -= size;
		_SizeOccupied -= size;
		return ptr;
	}
	else
		return -1;
}
