///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2010 by John Belcher.
///////////////////////////////////////////////////////////////////////////////

#include "targetver.h"
#include <windows.h>
#include <winuser.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
using namespace std;

struct MOUSESPEEDINFO
{
	MOUSESPEEDINFO(int spd = 10, bool accel = false) 
		: speed(spd)
		, accelLevel(accel?1:0)
		, accelThreshold1(6)
		, accelThreshold2(10) {}

	int speed; //1..20. Default=10
	int accelThreshold1; //First distance threshold. Default=6
	int accelThreshold2; //Second distance threshold. Default=10
	int accelLevel; //0=No acceleration, 1=2xThreshold, 2=4xThreshold. Default=0
	//Note: the order of the "accel" fields is significant.
};

HRESULT GetMouseSpeed(OUT MOUSESPEEDINFO &info)
{
	BOOL success = SystemParametersInfo(SPI_GETMOUSE, 0, reinterpret_cast<void*>(&info.accelThreshold1), 0);
	if (success)
		success = SystemParametersInfo(SPI_GETMOUSESPEED, 0, reinterpret_cast<void*>(&info.speed), 0);
	return (success ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
}
HRESULT SetMouseSpeed(const MOUSESPEEDINFO &info)
{
	BOOL success = SystemParametersInfo(SPI_SETMOUSE, 0, reinterpret_cast<void*>(const_cast<int*>(&info.accelThreshold1)), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
	if (success)
		success = SystemParametersInfo(SPI_SETMOUSESPEED, 0, reinterpret_cast<void*>(info.speed), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
	return (success ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
}

HRESULT processInput(MOUSESPEEDINFO& info, int argc, _TCHAR* argv[])
{
	LPTSTR end = NULL;
	if (argc > 1)
	{
		info.speed = _tcstol(argv[1], &end, 10);
		if (end == NULL || info.speed < 0 || info.speed > 20)
			return E_INVALIDARG;
	}
	if (argc > 2)
	{
		info.accelLevel = _tcstol(argv[2], &end, 10);
		if (end == NULL || info.accelLevel < 0 || info.accelLevel > 2)
			return E_INVALIDARG;
	}
	if (argc > 3)
	{
		if (argc <= 4)
			return E_INVALIDARG;

		info.accelThreshold1 = _tcstol(argv[3], &end, 10);
		if (end == NULL || info.accelThreshold1 < 1)
			return E_INVALIDARG;

		info.accelThreshold2 = _tcstol(argv[4], &end, 10);
		if (end == NULL || info.accelThreshold2 < info.accelThreshold1)
			return E_INVALIDARG;
	}
	if (argc > 5)
		return E_INVALIDARG;

	return S_OK;
}

int _tmain(int argc, _TCHAR* argv[])
{
	MOUSESPEEDINFO info;
	HRESULT hr = S_OK;
	if (argc > 1)
	{
		const MOUSESPEEDINFO defaults;
		hr = processInput(info, argc, argv);
		if (FAILED(hr))
		{
			cout << "Invalid input." << endl
				 << "  Usage: mousespd.exe [speed [accelLevel [threshold1 threshold2]]]" << endl
				 << "    speed: 1..20. Default=" << defaults.speed << "." << endl
				 << "    accelLevel: 0=Disabled, 1=2x acceleration, 2=4x acceleration. Default=" << defaults.accelLevel << "." << endl
				 << "    threshold1: Threshold for 2x acceleration. Default=" << defaults.accelThreshold1 << "." << endl
				 << "    threshold2: Threshold for 4x acceleration. Default=" << defaults.accelThreshold2 << "." << endl
				 << endl;
			return 1;
		}
		hr = SetMouseSpeed(info);
	}
	else
	{
		hr = GetMouseSpeed(info);
		if (SUCCEEDED(hr))
			cout << "Mouse speed=" << info.speed << endl
				 << "Acceleration level=" << info.accelLevel << endl
				 << "2x acceleration threshold=" << info.accelThreshold1 << endl
				 << "4x acceleration threshold=" << info.accelThreshold2 << endl
				 << endl;
	}

	if (FAILED(hr))
	{
		LPTSTR lpBuf = NULL;
		if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, 
				0, hr, 0, reinterpret_cast<LPTSTR>(&lpBuf), 0, NULL) == 0)
		{
			DWORD_PTR args[] = { (DWORD_PTR)hr };
			FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_ALLOCATE_BUFFER, 
				_T("Unknown error: %1!08lx!"), 0, 0, reinterpret_cast<LPTSTR>(&lpBuf), 0, (va_list*)args);
		}
		wcout << lpBuf << endl;
		LocalFree(lpBuf);
	}
	return hr;
}

