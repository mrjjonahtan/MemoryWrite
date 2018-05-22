// MemoryWrite.cpp : Defines the entry point for the console application.
//https://github.com/mrjjonahtan/MemoryWrite.git

#include "stdafx.h"
#include "PETools\PeToolsClass.h"

//var
BYTE *imageBuffer = NULL;
DWORD imageBase = 0;
DWORD sizeOfImage = 0;
DWORD lfanew = 0;

//fun
void free();
DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter);


int main()
{
	//::GetCurrentProcess();
	PeToolsClass petc;

	//get context
	HANDLE ThrdHnd = ::GetCurrentThread();
	CONTEXT Ctx;
	Ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(ThrdHnd, &Ctx);

	//get imagebase/sizeOfImage
	//imagebase
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(Ctx.Ebx + 8), &imageBase, 4, NULL);
	if (imageBase == 0)
	{
		return 0;
	}

	//lfanew
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageBase + 60), &lfanew, 4, NULL);
	if (lfanew == 0)
	{
		return 0;
	}

	//(32)sizeOfImage
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageBase + lfanew + 24 + 56), &sizeOfImage, 4, NULL);
	if (sizeOfImage == 0)
	{
		return 0;
	}


	//malloc
	imageBuffer = (BYTE*)malloc(sizeOfImage);
	if (imageBuffer == NULL)
	{
		return 0;
	}
	memset(imageBuffer, 0, sizeOfImage);

	//copy self all memory
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageBase), imageBuffer, sizeOfImage, NULL);

	//open process need inject
	DWORD pid = 0;
	HWND hwnd = ::FindWindow(TEXT("EVERYTHING"), TEXT("Everything"));//Everything EVERYTHING 计算器 SciCalc
	::GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0)
	{
		return 0;
	}
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);//(PID)

	if (hProcess == NULL)
	{
		return 0;
	}

	//application memory
	PVOID memoryLocation = VirtualAllocEx(hProcess, NULL, sizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (!memoryLocation)
	{
		return 0;
	}

	//repair relocation

	DWORD relocationRVA = petc.getDWValue(imageBuffer + lfanew + 24 + 96 + (5 * 8), 4);

	int value =  (DWORD)memoryLocation - imageBase;


	while (true)
	{
		DWORD RelocationRVA = petc.getDWValue((imageBuffer + relocationRVA), 4);
		DWORD sizeofBlock = petc.getDWValue((imageBuffer + relocationRVA + 4), 4);
		if (RelocationRVA == 0 && sizeofBlock == 0)
		{
			break;
		}
		petc.putData((imageBuffer + relocationRVA), (RelocationRVA + value), 4);
		relocationRVA = relocationRVA + sizeofBlock;
	}


	//write
	WriteProcessMemory(hProcess, memoryLocation, imageBuffer, sizeOfImage, NULL);

	//remote thread
	HANDLE hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((DWORD)ThreadProc + value), NULL, NULL, NULL);

	::WaitForSingleObject(hRemoteThread, INFINITE);
	::CloseHandle(hRemoteThread);
	hRemoteThread = NULL;


	free();
	printf("end");
	getchar();
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


DWORD WINAPI ThreadProc(
	_In_ LPVOID lpParameter
)
{
	OutputDebugString(L"\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=\n");
	return 0;
}
