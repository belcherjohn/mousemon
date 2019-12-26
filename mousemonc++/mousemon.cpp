// mousemon.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "resource.h"
#include "mousemon.h"
#include <shellapi.h>
#include <crtdbg.h>
#include <commctrl.h>
#pragma comment(lib,"Comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <dbt.h>

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

// Global Variables:
#define MAX_LOADSTRING 100
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HICON hLargeIcon = NULL, hSmallIcon = NULL;

#define WM_ICON_NOTIFY (WM_USER+1)

MOUSESPEEDINFO prevMouseSpeed;
TCHAR szMouclassEnumRegPath[] = L"HKLM\\SYSTEM\\CurrentControlSet\\services\\mouclass\\Enum";
TCHAR szDeviceEnumRegPath[] = L"HKLM\\SYSTEM\\CurrentControlSet\\Enum";


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass();
BOOL				InitInstance(int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				OnIconNotify(HWND, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	hInst = hInstance; // Store instance handle in our global variable

	hSmallIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	hLargeIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MOUSEMON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_SHARED);
	if (hSmallIcon == NULL)
		hSmallIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MOUSEMON));

	// Initialize global strings
	LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInst, IDC_MOUSEMON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass();

	// Perform application initialization:
	if (!InitInstance (nCmdShow))
		return FALSE;

	GetMouseSpeed(prevMouseSpeed);

	// Main message loop:
	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_MOUSEMON));
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass()
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= hLargeIcon ? hLargeIcon : hSmallIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MOUSEMON);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= hSmallIcon;
	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(int nCmdShow)
{
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, NULL);
	if (!hWnd)
		return FALSE;

	//ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CString dbgmsg;
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = WM_ICON_NOTIFY;
	nid.uFlags = 0;

	switch (message)
	{
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
			case ID_MOUSESETTINGS:
				{
					CStringA cmdline;
					if (cmdline.GetEnvironmentVariableW("SystemRoot"))
					{
						cmdline = "\"" + cmdline + "\\system32\\control.exe\" mouse";
						WinExec(cmdline, SW_NORMAL);
					}
				}
				break;

			case ID_ABOUT:
				{
					HWND existingWnd = GetWindow(hWnd, GW_ENABLEDPOPUP);
					if (existingWnd && existingWnd != hWnd)
						SetForegroundWindow(existingWnd);
					else
						DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				}
				break;

			case ID_EXIT:
				DestroyWindow(hWnd);
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

	//case WM_PAINT:
	//	hdc = BeginPaint(hWnd, &ps);
	//	// TODO: Add any drawing code here...
	//	EndPaint(hWnd, &ps);
	//	break;

	case WM_CREATE:
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		nid.uCallbackMessage = WM_ICON_NOTIFY;
		nid.hIcon = hSmallIcon;
		LoadString(hInst, IDS_APP_TITLE, nid.szTip, ARRAYSIZE(nid.szTip));
		Shell_NotifyIcon(NIM_ADD, &nid);
		break;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;

	case WM_ICON_NOTIFY:
		OnIconNotify(hWnd, wParam, lParam);
		break;

	case WM_SETTINGCHANGE:
		dbgmsg.Format(L"WM_SETTINGCHANGE(0x%04lx)\n", wParam);
		OutputDebugString(dbgmsg);
		if (wParam == 0 || wParam == SPI_SETMOUSE || wParam == SPI_SETMOUSESPEED)
			SetTimer(hWnd, WM_SETTINGCHANGE, 1000, NULL);
		break;

	case WM_DEVICECHANGE:
		dbgmsg.Format(L"WM_DEVICECHANGE(0x%04lx)\n", wParam);
		OutputDebugString(dbgmsg);
		if (wParam == DBT_DEVNODES_CHANGED)
		{
			KillTimer(hWnd, WM_SETTINGCHANGE);
			SetTimer(hWnd, WM_DEVICECHANGE, 2000, NULL);
		}
		break;

	case WM_TIMER:
		{
			dbgmsg.Format(L"WM_TIMER(%d)\n", wParam);
			OutputDebugString(dbgmsg);

			KillTimer(hWnd, wParam);

			nid.uFlags = NIF_INFO;
			nid.dwInfoFlags = NIIF_USER | NIIF_NOSOUND;// | NIIF_RESPECT_QUIET_TIME;// | NIF_REALTIME;
			StringCchCopy(nid.szInfoTitle, _countof(nid.szInfoTitle), CString((LPCWSTR)MAKEINTRESOURCE(IDS_APP_TITLE)));
			nid.hIcon = hSmallIcon;
//			if (hLargeIcon != NULL)
//				nid.hBalloonIcon = hLargeIcon, nid.dwInfoFlags |= NIIF_LARGE_ICON;

			MOUSESPEEDINFO curMouseSpeed;
			if (FAILED(GetMouseSpeed(curMouseSpeed)))
			{
				nid.szInfo[0] = 0;
				Shell_NotifyIcon(NIM_MODIFY, &nid);
				nid.dwInfoFlags = NIIF_ERROR;
				StringCchCopy(nid.szInfo, _countof(nid.szInfo), CString((LPCWSTR)MAKEINTRESOURCE(IDS_E_CANT_READ_MOUSE_SPEED)));
				Shell_NotifyIcon(NIM_MODIFY, &nid);
				break;
			}
			if (memcmp(&curMouseSpeed, &prevMouseSpeed, sizeof(prevMouseSpeed)) == 0)
				break; //no change

			CString msg;
			if (wParam == WM_DEVICECHANGE)
			{
				if (FAILED(SetMouseSpeed(prevMouseSpeed)))
				{
					nid.dwInfoFlags = NIIF_ERROR;
					StringCchCopy(nid.szInfo, _countof(nid.szInfo), CString((LPCWSTR)MAKEINTRESOURCE(IDS_E_CANT_SET_MOUSE_SPEED)));
					Shell_NotifyIcon(NIM_MODIFY, &nid);
					break;
				}
				GetMouseSpeed(prevMouseSpeed);
				msg = CString((LPCWSTR)MAKEINTRESOURCE(IDS_RESTORED_MOUSE_SPEED));
			}
			else if (wParam == WM_SETTINGCHANGE)
			{
				prevMouseSpeed = curMouseSpeed;
				msg = CString((LPCWSTR)MAKEINTRESOURCE(IDS_UPDATED_MOUSE_SPEED));
			}
			else
			{
				_ASSERT(0);
				break;
			}

			CString status;
			status.FormatMessage(IDS_NOTIFY_ICON_STATUS_MSG, 
				(LPCWSTR)msg, 
				prevMouseSpeed.speed, 
				prevMouseSpeed.accelLevel, 
				prevMouseSpeed.accelThreshold1, 
				prevMouseSpeed.accelThreshold2);
			OutputDebugString(status);
			OutputDebugString(L"\n");

			nid.szInfo[0] = 0;
			Shell_NotifyIcon(NIM_MODIFY, &nid);
			StringCchCopy(nid.szInfo, _countof(nid.szInfo), status);
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void OnIconNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int eventId = (int)lParam;
	int iconId  = (int)wParam;
	_ASSERT(iconId == WM_ICON_NOTIFY);

	const int DefaultCommandId = ID_MOUSESETTINGS;

	switch(eventId)
	{
	case WM_RBUTTONUP:
		OutputDebugString(L"OnIconNotify(WM_RBUTTONUP)\n");
		{
			HMENU hmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_CONTEXT_MENU));
			HMENU hmenuTrackPopup = GetSubMenu(hmenu, 0);
			if (hmenuTrackPopup != NULL && SetForegroundWindow(hWnd))
			{
				SetMenuDefaultItem(hmenuTrackPopup, DefaultCommandId, FALSE);
				POINT pt;
	 			GetCursorPos(&pt);
				TrackPopupMenu(hmenuTrackPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL); 
				PostMessage(hWnd, WM_NULL, 0, 0);
			}
			DestroyMenu(hmenu);
		}
		break;

	case WM_LBUTTONDBLCLK:
		OutputDebugString(L"OnIconNotify(WM_LBUTTONDBLCLK)\n");
		PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(DefaultCommandId, 0), 0);
		break;
	case NIN_BALLOONSHOW:
		OutputDebugString(L"OnIconNotify(NIN_BALLOONSHOW)\n");
		break;
	case NIN_BALLOONHIDE:
		OutputDebugString(L"OnIconNotify(NIN_BALLOONHIDE)\n");
		break;
	case NIN_BALLOONTIMEOUT:
		OutputDebugString(L"OnIconNotify(NIN_BALLOONTIMEOUT)\n");
		break;
	case NIN_BALLOONUSERCLICK:
		OutputDebugString(L"OnIconNotify(NIN_BALLOONUSERCLICK)\n");
		break;
	default:
		if (!(WM_MOUSEFIRST <= eventId  && eventId <= WM_MOUSELAST))
		{
			CString dbgmsg;
			dbgmsg.Format(L"OnIconNotify(%d)\n", eventId);
			OutputDebugString(dbgmsg);
		}
		break;
	}
}
