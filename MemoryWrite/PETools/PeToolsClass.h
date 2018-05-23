#pragma once
class PeToolsClass
{
public:
	PeToolsClass();
	~PeToolsClass();

	//void getValue(BYTE *pointerValue, int number, TCHAR *tvlue);
	DWORD getDWValue(BYTE *pointerValue, int number);
	DWORD getPELocation(BYTE *pointerValue);
	/*DWORD getOptionSizeValue(BYTE *pointerValue);
	DWORD getSectionNumber(BYTE *pointerValue);*/
	void getCharPointer(BYTE *pointerValue, TCHAR *tvlue, int max);
	/*DWORD getAlignData(DWORD data,DWORD alig);
	DWORD rvaTofoa(BYTE *pointerValue,DWORD RVA);
	DWORD foaTorva(BYTE *pointerValue, DWORD FOA);*/
	DWORD getApplicationSize(BYTE *pointerValue);
	void putData(BYTE *pointer, DWORD value, DWORD number);
};

