#include "../Udis86Wrapper/Udis86Wrapper.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include "../udis86-1.7.2/Include/udis86.h"
#pragma comment(lib, "../udis86-1.7.2/Lib/x86/libudis86.lib")
using namespace std;

typedef struct
{
	LPCVOID addr;
	byte firstByte;
	LPCVOID pNext;
} Breakpoint, * pBreakpoint;

constexpr auto TargetFile = L"../Debug/CppConsoleApp.exe";
byte* entryAddr = NULL;
pBreakpoint g_bpHead = NULL, g_bpEnd = NULL;

void setBreakPoint(HANDLE hProcess, LPCVOID addr, byte firstByte)
{
	auto bp = (pBreakpoint)malloc(sizeof(Breakpoint));

	if (bp != NULL)
	{
		bp->addr = addr;
		bp->firstByte = firstByte;
		bp->pNext = NULL;

		if (g_bpHead == NULL)
		{
			g_bpHead = bp;
			g_bpEnd = bp;
		}
		else
		{
			g_bpEnd->pNext = bp;
			g_bpEnd = bp;
		}

		char int3 = 0xCC;
		SIZE_T written;
		WriteProcessMemory(hProcess, (LPVOID)addr, &int3, 1, &written);
	}
}

void OnProcessCreated(DEBUG_EVENT debugEvent)
{
	auto debugInfo = &debugEvent.u.CreateProcessInfo;
	byte buf[88];
	memset(buf, 0, sizeof(buf));

	SIZE_T read;
	ReadProcessMemory(debugInfo->hProcess, (LPCVOID)entryAddr, buf, sizeof(buf), &read);
	Disassemble(entryAddr, sizeof(buf), buf);
	cout << endl;

	setBreakPoint(debugInfo->hProcess, (byte*)0x4113A7, buf[0x4113A7 - (int)entryAddr]);

	ReadProcessMemory(debugInfo->hProcess, (LPCVOID)entryAddr, buf, sizeof(buf), &read);
	Disassemble(entryAddr, sizeof(buf), buf);
	cout << endl;

	cout << (string)"on process created." << endl;
}

void OnException(DEBUG_EVENT debugEvent, PROCESS_INFORMATION* pi, PDWORD pContiniueStatus)
{
	auto debugInfo = &debugEvent.u.CreateProcessInfo;
	auto exceptionInfo = debugEvent.u.Exception;

	if (exceptionInfo.dwFirstChance)
	{
		cout << (string)"On exception at " << exceptionInfo.ExceptionRecord.ExceptionAddress << endl;
		pBreakpoint tmp = g_bpHead;
		for (; tmp != NULL; tmp = (pBreakpoint)tmp->pNext)
		{
			if (tmp->addr == exceptionInfo.ExceptionRecord.ExceptionAddress)
			{
				SIZE_T written;
				WriteProcessMemory(pi->hProcess, (LPVOID)tmp->addr, &tmp->firstByte, 1, &written);

				CONTEXT ctmp;
				ctmp.ContextFlags = CONTEXT_CONTROL;
				GetThreadContext(pi->hThread, &ctmp);
				ctmp.Eip--;
				ctmp.EFlags |= 0x100;		// TF = 1
				SetThreadContext(pi->hThread, &ctmp);
				break;
			}
		}
		*pContiniueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	else *pContiniueStatus = DBG_CONTINUE;
}

int main()
{
	cout << (string)("CppDebuggerApp...") << endl;

	HANDLE hFile = CreateFileW(TargetFile, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "Open target file failed." << endl;
		return -1;
	}

	DWORD dwRead;
	DWORD dwSize = GetFileSize(hFile, &dwRead);
	byte* buf = (byte*)malloc(dwSize);
	if (buf)
	{
		memset(buf, 0, (size_t)dwSize);
		auto fileOpened = ReadFile(hFile, buf, dwSize, &dwRead, 0);
		if (fileOpened)
		{
			IMAGE_NT_HEADERS* stNt = (IMAGE_NT_HEADERS*)((byte*)buf + ((IMAGE_DOS_HEADER*)buf)->e_lfanew);
			entryAddr = (byte*)(stNt->OptionalHeader.ImageBase + stNt->OptionalHeader.AddressOfEntryPoint);
		}
		free(buf);
	}
	CloseHandle(hFile);

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessW(TargetFile, NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		cout << L"Create Process %s failed." << TargetFile << endl;
		return -1;
	}

	DEBUG_EVENT debugEvent;
	BOOL waitEvent = TRUE;
	DWORD continiueStatus = DBG_EXCEPTION_NOT_HANDLED;
	while (WaitForDebugEvent(&debugEvent, INFINITE))
	{
		switch (debugEvent.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			OnProcessCreated(debugEvent);
			break;

		case EXCEPTION_DEBUG_EVENT:
			OnException(debugEvent, &pi, &continiueStatus);
			break;

		default:
			break;
		}
		if (waitEvent == TRUE)
		{
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continiueStatus);
		}
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
