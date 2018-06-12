// MemoryWrite.cpp : Defines the entry point for the console application.
//https://github.com/mrjjonahtan/MemoryWrite.git

#include "stdafx.h"
#include "PETools\PeToolsClass.h"

//var
BYTE *imageBuffer = NULL;
DWORD imageBase = 0;
DWORD sizeOfImage = 0;
DWORD lfanew = 0;
bool imageFlage = false;

//fun
void free();
void repairIAT(DWORD imageValue);
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

	int value = 0;

	if (imageBase > (DWORD)memoryLocation)
	{
		value = imageBase - (DWORD)memoryLocation;
	}
	else if (imageBase < (DWORD)memoryLocation)
	{
		value = (DWORD)memoryLocation - imageBase;
		imageFlage = true;
	}

	//change image
	//petc.putData(imageBuffer + lfanew + 24 + 28, (DWORD)memoryLocation, 4);

	if (relocationRVA != 0)
	{
		while (true)
		{
			DWORD RelocationRVA = petc.getDWValue((imageBuffer + relocationRVA), 4);
			DWORD sizeofBlock = petc.getDWValue((imageBuffer + relocationRVA + 4), 4);
			if (RelocationRVA == 0 && sizeofBlock == 0)
			{
				break;
			}

			if (imageFlage)
			{
				petc.putData((imageBuffer + relocationRVA), (RelocationRVA + value), 4);
			}
			else
			{
				petc.putData((imageBuffer + relocationRVA), (RelocationRVA - value), 4);
			}

			relocationRVA = relocationRVA + sizeofBlock;
		}
	}

	//repairIAT(0x400000);

	//file text
	/*FILE *fp = NULL;
	fopen_s(&fp, "test.exe", "wb");
	fwrite(imageBuffer, sizeOfImage, 1, fp);
	fclose(fp);*/


	//write
	WriteProcessMemory(hProcess, memoryLocation, imageBuffer, sizeOfImage, NULL);

	//DWORD sd = (DWORD)ThreadProc;

	//remote thread
	HANDLE hRemoteThread = NULL;
	//hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, (LPVOID)memoryLocation, NULL, NULL);

	if (imageFlage)
	{
		hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)(((DWORD)ThreadProc) + value), (LPVOID)memoryLocation, NULL, NULL);
	}
	else
	{
		hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)(((DWORD)ThreadProc) - value), (LPVOID)memoryLocation, NULL, NULL);
	}


	::WaitForSingleObject(hRemoteThread, INFINITE);
	DWORD lpExitCode = -1;
	::GetExitCodeThread(hRemoteThread, &lpExitCode);
	::CloseHandle(hRemoteThread);
	hRemoteThread = NULL;

	DWORD asda = (DWORD)MessageBoxW;

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

	DWORD imageValue = (DWORD)lpParameter;
	bool imageFlageT = false;
	int valueT = 0;
	//
	PIMAGE_DOS_HEADER pDos = NULL;
	PIMAGE_NT_HEADERS pNtHeader = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = NULL;

	//dos
	pDos = (PIMAGE_DOS_HEADER)imageValue;
	pNtHeader = (PIMAGE_NT_HEADERS)(imageValue + pDos->e_lfanew);
	pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(imageValue + pNtHeader->OptionalHeader.DataDirectory[1].VirtualAddress);
	if (pDos == 0 || pNtHeader == 0)
	{
		return 0;
	}

	DWORD ImageBaseT = pNtHeader->OptionalHeader.ImageBase;

	if (ImageBaseT > imageValue)
	{
		valueT = ImageBaseT - imageValue;
	}
	else if (ImageBaseT < imageValue)
	{
		valueT = imageValue - ImageBaseT;
		imageFlageT = true;
	}

	pNtHeader->OptionalHeader.ImageBase = imageValue;
	PDWORD pFuncAddr = NULL;

	//IAT
	while (pImportDescriptor->FirstThunk != 0)
	{
		pFuncAddr = (PDWORD)(imageValue + pImportDescriptor->FirstThunk);
		while (*pFuncAddr)
		{
			//if (0x76e082c0  == *pFuncAddr) {
			//	//定义函数指针
			//	typedef int (WINAPI *PFNMESSAGEBOX)(HWND, LPCWSTR, LPCWSTR, UINT);

			//	//执行真正的函数
			//	int ret = ((PFNMESSAGEBOX)0x76e082c0)(NULL, L"remote", L"remote", MB_OK);
			//}
			/*if (imageFlageT)
			{
				*pFuncAddr = (*pFuncAddr + valueT);
			}
			else
			{
				*pFuncAddr = (*pFuncAddr - valueT);
			}*/
			*pFuncAddr = (0x76e082c0);
			pFuncAddr++;
		}
		pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pImportDescriptor + sizeof(_IMAGE_IMPORT_DESCRIPTOR));
	}

	//MessageBoxW(NULL, L"remote", L"remote", MB_OK);
	//OutputDebugString(L"\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=\n");
	//OutputDebugString(L"\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=\n");

	return (DWORD)MessageBoxW;
}
