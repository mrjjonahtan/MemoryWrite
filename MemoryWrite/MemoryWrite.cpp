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
	//DWORD image = petc.getDWValue(imageBuffer + lfanew + 24 + 28, 4);
	petc.putData(imageBuffer + lfanew + 24 + 28, (DWORD)memoryLocation, 4);

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


	//file text
	/*FILE *fp = NULL;
	fopen_s(&fp, "test.exe", "wb");
	fwrite(imageBuffer, sizeOfImage, 1, fp);
	fclose(fp);*/


	//write
	WriteProcessMemory(hProcess, memoryLocation, imageBuffer, sizeOfImage, NULL);

	//remote thread
	HANDLE hRemoteThread = NULL;
	hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, (LPVOID)memoryLocation, NULL, NULL);

	/*if (imageFlage)
	{
		hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)(((DWORD)ThreadProc) + value), (LPVOID)memoryLocation, NULL, NULL);
	}
	else
	{
		hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)(((DWORD)ThreadProc) - value), (LPVOID)memoryLocation, NULL, NULL);
	}*/


	::WaitForSingleObject(hRemoteThread, INFINITE);
	DWORD lpExitCode = -1;
	::GetExitCodeThread(hRemoteThread, &lpExitCode);
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
	
	//DWORD imageBase = (DWORD)lpParameter;
	repairIAT(imageBase);

	MessageBox(NULL, L"remote", L"remote", MB_OK);
	OutputDebugString(L"\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=\n");
	//OutputDebugString(L"\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=\n");
	return 9;
}

//loading iat
void repairIAT(DWORD imageValue)
{
	unsigned long directoryLocat = 0;
	PeToolsClass petc;

	//
	DWORD platform = 0;
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + lfanew + 4), &platform, 4, NULL);
	if (platform == 0x014C)
	{
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + lfanew + 24 + 96), &directoryLocat, 4, NULL);
	}
	else if (platform == 0x8664)
	{
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + lfanew + 24 + 112), &directoryLocat, 4, NULL);
	}


	TCHAR *temporaryBUffer = NULL;
	temporaryBUffer = (TCHAR*)malloc(sizeof(TCHAR) * 0x50);
	if (temporaryBUffer == NULL)
	{
		return;
	}
	memset(temporaryBUffer, 0, sizeof(TCHAR) * 0x50);

	DWORD importRVA = 0;
	ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + directoryLocat + 8), &importRVA, 4, NULL);
	if (importRVA == 0)
	{
		return;
	}
	//DWORD importFOA = petc.rvaTofoa(pointer, importRVA);

	for (int i = 0; ; i++)
	{
		DWORD originalFirstThunkRVA = 0;
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + importRVA + (i * 20)), &originalFirstThunkRVA, 4, NULL);

		DWORD firstThunkRVA = 0;
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + importRVA + 16 + (i * 20)), &firstThunkRVA, 4, NULL);

		if (originalFirstThunkRVA == 0 || firstThunkRVA == 0)
		{
			break;
		}
		DWORD timeDateStamp = 0;
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + importRVA + 4 + (i * 20)), &timeDateStamp, 4, NULL);

		if (timeDateStamp == -1)
		{
			//descriptor


			continue;
		}
		//DWORD nameRVA = petc.getDWValue((pei->pointer + importFOA + 12 + (i * 20)), 4);
		//DWORD nameFOA = petc.rvaTofoa(pointer, petc.getDWValue((pointer + importFOA + 12 + (i * 20)), 4));
		DWORD nameRVA = 0;
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + importRVA + 12 + (i * 20)), &nameRVA, 4, NULL);

		memset(temporaryBUffer, 0, sizeof(TCHAR) * 0x50);
		//wsprintf(temporaryBUffer, L"%08X", firstThunkRVA);
		//wsprintf(temporaryBUffer + 9, L"%08X", timeDateStamp);
		//wsprintf(temporaryBUffer + 18, L"%08X", originalFirstThunkRVA);
		//petc.getCharPointer((pointer + nameRVA), temporaryBUffer, 0);
		ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + nameRVA), temporaryBUffer, 0x50, NULL);

		//IAT
		//OutputDebugString((temporaryBUffer+27));

		for (int i = 0; ; i++)
		{
			DWORD thunkdataINT = 0;
			ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + firstThunkRVA + (i * 4)), &thunkdataINT, 4, NULL);

			if (thunkdataINT == 0)
			{
				break;
			}
			//int sd = thunkdataINT & 0x80000000;
			if ((thunkdataINT & 0x80000000))
			{

				HMODULE hLoadDll = LoadLibrary(temporaryBUffer);

				if (!hLoadDll)
				{
					wchar_t hint[0x50] = { 0 };
					swprintf(hint, 0x50, L"DLL(%s)加载失败\n请检查环境DLL是否丢失，\n程序将尝试继续加载其他DLL并启动程序。", temporaryBUffer);
					MessageBox(NULL, hint, L"加载DLL提示", MB_OK);
				}

				DWORD iat = (DWORD)::GetProcAddress(hLoadDll, (LPCSTR)(thunkdataINT & 0x7fffffff));

				//petc.putData(pointer + firstThunkRVA + (i * 4), iat, 4);
				WriteProcessMemory(::GetCurrentProcess(), (LPVOID)(imageValue + firstThunkRVA + (i * 4)), &iat, 4, NULL);
			}
			else
			{

				//DWORD hint = petc.getDWValue(pointer + thunkdataINT, 2);

				//wchar_t wfunName[0x30] = { 0 };
				char funName[0x40] = { 0 };

				//petc.getCharPointer(pointer + thunkdataINT + 2, wfunName, 0);
				ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)(imageValue + thunkdataINT + 2), funName, 0x40, NULL);

				//sprintf_s(funName, "%ws", wfunName);

				DWORD iat = (DWORD)::GetProcAddress(LoadLibrary(temporaryBUffer), funName);

				//petc.putData(pointer + firstThunkRVA + (i * 4), iat, 4);
				WriteProcessMemory(::GetCurrentProcess(), (LPVOID)(imageValue + firstThunkRVA + (i * 4)), &iat, 4, NULL);

			}

		}

	}
	if (temporaryBUffer != NULL)
	{
		free(temporaryBUffer);
		temporaryBUffer = NULL;
	}
}
