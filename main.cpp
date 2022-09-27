#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <strsafe.h>

#include <process.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "user32.lib")

HWND GetConsoleHwnd() {
	WCHAR newTitle[MAX_PATH];
	WCHAR oldTitle[MAX_PATH];

	GetConsoleTitle(oldTitle, MAX_PATH);

	StringCchPrintf(newTitle, MAX_PATH, L"EmbedWindow %d %d", 
			GetTickCount(), GetCurrentProcessId()
	);
	SetConsoleTitle(newTitle);
	Sleep(40);
	HWND found = FindWindow(NULL, newTitle);
	SetConsoleTitle(oldTitle);
	return found;
}

void SetChildStyle(HWND sub) {
	LONG style = GetWindowLong(sub, GWL_STYLE);
	style |= WS_CHILD;
	// Remove titlebar from window
	style &= ~(
		WS_CAPTION | 
		WS_THICKFRAME | 
		WS_MINIMIZEBOX | 
		WS_MAXIMIZEBOX | 
		WS_SYSMENU
	);
	SetWindowLong(sub, GWL_STYLE, style);
}

void EmbedWindow(HWND sub, HWND dom) {
	SetParent(sub, dom);
	SetChildStyle(sub);
	SetWindowPos(sub, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

int wmain(int argc, WCHAR** argv) {
	LPCWSTR cmdName = L"C:\\Windows\\System32\\cmd.exe";

	if (argc < 4) {
		std::wcerr << L"usage: " << argv[0] 
			   << " \"class name\" \"window title\" x y command\n";
		return 1;
	}

	HWND console = GetConsoleHwnd();
	HWND embed = FindWindow(argv[1], argv[2]);
	int x = _wtoi(argv[3]), y = _wtoi(argv[4]);

	EmbedWindow(console, embed);
	SetWindowPos(console, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	std::vector<LPCWSTR> v(argv+5, argv+argc);
	v.insert(v.begin(), L"/c");
	v.insert(v.begin(), cmdName);
	v.push_back(NULL);

	LPCWSTR* prgArgv = v.data(); 
	_wexecv(cmdName, prgArgv);

	// exec will take over our process. if it didn't, there's an error.
	int e = errno;
	std::wcerr << L"_wexecv returned error " << e << L'\n';
	return e;
}


