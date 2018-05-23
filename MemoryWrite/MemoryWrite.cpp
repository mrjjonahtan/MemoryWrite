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
void repairIAT(BYTE *pointer);
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

//loading iat
void repairIAT(BYTE *pointer)
{
	unsigned long directoryLocat = 0;
	PeToolsClass petc;

	//
	DWORD platform = petc.getApplicationSize(pointer);
	if (platform == 0x014C)
	{
		directoryLocat = petc.getDWValue((pointer + 60), 4) + 24 + 96;
	}
	else if (platform == 0x8664)
	{
		directoryLocat = petc.getDWValue((pointer + 60), 4) + 24 + 112;
		//
		MessageBox(NULL, L"64程序进程加载，还在编码中，请稍后...", L"提示", MB_OK);
		return;
	}


	TCHAR *temporaryBUffer = NULL;
	temporaryBUffer = (TCHAR*)malloc(sizeof(TCHAR) * 0x100);
	if (temporaryBUffer == NULL)
	{
		return;
	}
	memset(temporaryBUffer, 0, sizeof(TCHAR) * 0x50);

	DWORD importRVA = petc.getDWValue((pointer + directoryLocat + 8), 4);
	if (importRVA == 0)
	{
		return;
	}
	//DWORD importFOA = petc.rvaTofoa(pointer, importRVA);

	for (int i = 0; ; i++)
	{
		DWORD originalFirstThunkRVA = petc.getDWValue((pointer + importRVA + (i * 20)), 4);
		DWORD firstThunkRVA = petc.getDWValue((pointer + importRVA + 16 + (i * 20)), 4);
		if (originalFirstThunkRVA == 0 || firstThunkRVA == 0)
		{
			break;
		}
		DWORD timeDateStamp = petc.getDWValue((pointer + importRVA + 4 + (i * 20)), 4);
		if (timeDateStamp == -1)
		{
			//descriptor


			continue;
		}
		//DWORD nameRVA = petc.getDWValue((pei->pointer + importFOA + 12 + (i * 20)), 4);
		//DWORD nameFOA = petc.rvaTofoa(pointer, petc.getDWValue((pointer + importFOA + 12 + (i * 20)), 4));
		DWORD nameRVA = petc.getDWValue((pointer + importRVA + 12 + (i * 20)), 4);

		memset(temporaryBUffer, 0, sizeof(TCHAR) * 0x50);
		//wsprintf(temporaryBUffer, L"%08X", firstThunkRVA);
		//wsprintf(temporaryBUffer + 9, L"%08X", timeDateStamp);
		//wsprintf(temporaryBUffer + 18, L"%08X", originalFirstThunkRVA);
		petc.getCharPointer((pointer + nameRVA), temporaryBUffer, 0);

		//IAT
		//OutputDebugString((temporaryBUffer+27));

		for (int i = 0; ; i++)
		{
			DWORD thunkdataINT = petc.getDWValue(pointer + firstThunkRVA + (i * 4), 4);
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

				petc.putData(pointer + firstThunkRVA + (i * 4), iat, 4);

			}
			else
			{

				//DWORD hint = petc.getDWValue(pointer + thunkdataINT, 2);

				wchar_t wfunName[0x30] = { 0 };
				char funName[0x40] = { 0 };

				petc.getCharPointer(pointer + thunkdataINT + 2, wfunName, 0);

				sprintf_s(funName, "%ws", wfunName);

				DWORD iat = (DWORD)::GetProcAddress(LoadLibrary(temporaryBUffer), funName);

				petc.putData(pointer + firstThunkRVA + (i * 4), iat, 4);

			}

		}

	}
	if (temporaryBUffer != NULL)
	{
		free(temporaryBUffer);
		temporaryBUffer = NULL;
	}
}
