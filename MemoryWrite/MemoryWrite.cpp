// MemoryWrite.cpp : Defines the entry point for the console application.
//https://github.com/mrjjonahtan/MemoryWrite.git

#include "stdafx.h"
#include <Windows.h>

//var
BYTE *imageBuffer = NULL;
DWORD imageBase = 0;

//fun
void free();


int main()
{
	//::GetCurrentProcess();

	//get context
	HANDLE ThrdHnd = ::GetCurrentThread();
	CONTEXT Ctx;
	Ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(ThrdHnd, &Ctx);
	//get imagebase
	imageBase = Ctx.Ebx + 8;
	if (!imageBase)
	{
		return 0;
	}

	//malloc
	imageBuffer = (BYTE*)malloc(imageBase);
	if (imageBuffer == NULL)
	{
		return 0;
	}
	memset(imageBuffer, 0, imageBase);


	free();
    return 0;
}

void free()
{
	if (imageBuffer != NULL)
	{
		free(imageBuffer);
		imageBuffer = NULL;
	}
}
