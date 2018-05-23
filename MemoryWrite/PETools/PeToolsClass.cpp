#include "..\stdafx.h"
#include "PeToolsClass.h"


PeToolsClass::PeToolsClass()
{
}


PeToolsClass::~PeToolsClass()
{
}
//
//void PeToolsClass::getValue(BYTE *pointerValue, int number, TCHAR *tvlue)
//{
//	char *vaby = NULL;
//	vaby = (char*)malloc(0x200);
//	if (vaby == NULL)
//	{
//		return;
//	}
//	for (int i = number - 1, j = 0; i >= 0; i--, j++)
//	{
//		memset(vaby, 0, 0x200);
//		DWORD valueTem = *(pointerValue + i);
//		sprintf_s(vaby, 0x200, "%02X", valueTem);
//		for (int k = 0; *(vaby + k) != 0; k++)
//		{
//			*(tvlue + j) = *(vaby + k);
//			j++;
//		}
//		j--;
//	}
//	if (vaby != NULL) {
//		free(vaby);
//	}
//}

DWORD PeToolsClass::getDWValue(BYTE *pointerValue, int number)
{
	DWORD revalue = 0;
	for (int i = (number - 1); i >= 0; i--)
	{
		revalue = (revalue << 8) + *(pointerValue + i);
	}
	return revalue;
}
//
//DWORD PeToolsClass::getOptionSizeValue(BYTE *pointerValue)
//{
//	DWORD revalue = 0;
//	revalue = getDWValue((pointerValue + getPELocation(pointerValue) + 4 + 16), 2);
//	return revalue;
//}

DWORD PeToolsClass::getPELocation(BYTE *pointerValue)
{
	DWORD revalue = 0;
	revalue = getDWValue((pointerValue + 60), 4);
	return revalue;
}

//DWORD PeToolsClass::getSectionNumber(BYTE *pointerValue)
//{
//	DWORD revalue = 0;
//	revalue = getDWValue((pointerValue + getPELocation(pointerValue) + 4 + 2), 2);
//	return revalue;
//}
//
void PeToolsClass::getCharPointer(BYTE *pointerValue, TCHAR *tvlue, int max)
{
	if (max == 0)
	{
		for (int i = 0; pointerValue[i] != 0; i++)
		{
			tvlue[i] = pointerValue[i];
		}
	}
	else
	{
		for (int i = 0; pointerValue[i] != 0 && i < max; i++)
		{
			tvlue[i] = pointerValue[i];
		}
	}

}
//
//DWORD PeToolsClass::rvaTofoa(BYTE *pointerValue, DWORD RVA)
//{
//	struct mSection {
//		union
//		{
//			DWORD PhysicalAddress;
//			DWORD virtualSize;
//		} Misc;
//		DWORD virtualAddress;
//		DWORD sizeOfRawData;
//		DWORD pointertorawdata;
//	};
//	DWORD rtf = 0;
//	DWORD pelocat = getPELocation(pointerValue);
//	DWORD sizeOfHeaders = getDWValue((pointerValue + pelocat + 24 + 60), 4);
//	if (RVA > sizeOfHeaders)
//	{
//		DWORD optionSize = getOptionSizeValue(pointerValue);
//		int snumber = getSectionNumber(pointerValue);
//		mSection section[10] = { 0 };
//
//		for (int i = 0; i < snumber; i++)
//		{
//			DWORD locat = i * 40;
//			section[i].Misc.virtualSize = getDWValue((pointerValue + pelocat + optionSize + 24 + 8 + locat), 4);
//			section[i].virtualAddress = getDWValue((pointerValue + pelocat + optionSize + 24 + 12 + locat), 4);
//			section[i].sizeOfRawData = getDWValue((pointerValue + pelocat + optionSize + 24 + 16 + locat), 4);
//			section[i].pointertorawdata = getDWValue((pointerValue + pelocat + optionSize + 24 + 20 + locat), 4);
//		}
//
//		for (int i = 0; i < snumber; i++)
//		{
//			if (RVA >= section[i].virtualAddress && RVA < (getAlignData(section[i].Misc.virtualSize, 0x1000) + section[i].virtualAddress))
//			{
//				rtf = (RVA - section[i].virtualAddress) + section[i].pointertorawdata;
//			}
//		}
//	}
//	else
//	{
//		rtf = RVA;
//	}
//	return rtf;
//}
//
//DWORD PeToolsClass::foaTorva(BYTE *pointerValue, DWORD FOA)
//{
//	struct mSection {
//		union
//		{
//			DWORD PhysicalAddress;
//			DWORD virtualSize;
//		} Misc;
//		DWORD virtualAddress;
//		DWORD sizeOfRawData;
//		DWORD pointertorawdata;
//	};
//	DWORD ftr = 0;
//	DWORD pelocat = getPELocation(pointerValue);
//	DWORD optionSize = getOptionSizeValue(pointerValue);
//	int snumber = getSectionNumber(pointerValue);
//
//	mSection section[10] = { 0 };
//
//	for (int i = 0; i < snumber; i++)
//	{
//		DWORD locat = i * 40;
//		section[i].Misc.virtualSize = getDWValue((pointerValue + pelocat + optionSize + 24 + 8 + locat), 4);
//		section[i].virtualAddress = getDWValue((pointerValue + pelocat + optionSize + 24 + 12 + locat), 4);
//		section[i].sizeOfRawData = getDWValue((pointerValue + pelocat + optionSize + 24 + 16 + locat), 4);
//		section[i].pointertorawdata = getDWValue((pointerValue + pelocat + optionSize + 24 + 20 + locat), 4);
//	}
//	for (int i = 0; i < snumber; i++)
//	{
//		if (FOA >= section[i].pointertorawdata && FOA < section[i].sizeOfRawData + section[i].pointertorawdata)
//		{
//			ftr = (FOA - section[i].pointertorawdata) + section[i].virtualAddress;
//		}
//	}
//	return ftr;
//}
//
//DWORD PeToolsClass::getAlignData(DWORD data, DWORD alig)
//{
//	DWORD revalue = 0;
//	revalue = data % alig;
//	if (revalue != 0)
//	{
//		revalue = (data - revalue) + alig;
//	}
//	else
//	{
//		revalue = data;
//	}
//	return revalue;
//}

DWORD PeToolsClass::getApplicationSize(BYTE *pointerValue)
{
	DWORD pelocat = getPELocation(pointerValue);
	DWORD reValue = getDWValue((pointerValue + pelocat + 4), 2);
	return reValue;
}

void PeToolsClass::putData(BYTE *pointer, DWORD value, DWORD number)
{
	switch (number)
	{
	case 1:
		*pointer = (value << 24) >> 24;
		break;
	case 2:
		*pointer = (value << 24) >> 24;
		*(pointer + 1) = (value << 16) >> 24;
		break;
	case 3:
		*pointer = (value << 24) >> 24;
		*(pointer + 1) = (value << 16) >> 24;
		*(pointer + 2) = (value << 8) >> 24;
		break;
	case 4:
		*pointer = (value << 24) >> 24;
		*(pointer + 1) = (value << 16) >> 24;
		*(pointer + 2) = (value << 8) >> 24;
		*(pointer + 3) = value >> 24;
		break;
	}
}
