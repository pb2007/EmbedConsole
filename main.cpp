#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <strsafe.h>

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

	std::vector<std::wstring> v(argv+5, argv+argc);
	v.insert(v.begin(), L"/c");
	v.insert(v.begin(), cmdName);

	std::wstring tmp;
	for (const auto& p : v) tmp += p + L' ';
	LPWSTR args = tmp.data();

	STARTUPINFO si { .cb = sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };

	if (!CreateProcess(
		cmdName, args,
		NULL, NULL, false, 0, NULL, NULL,
		&si, &pi
	)) {
		DWORD e = GetLastError();
		std::wcerr << L"CreateProcess error: "
			   << std::hex << e;
		return e;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}


