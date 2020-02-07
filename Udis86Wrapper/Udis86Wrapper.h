#pragma once
#include <Windows.h>

extern "C" __declspec(dllexport) void __stdcall Disassemble(LPCVOID addr, SIZE_T size, LPCVOID buf);
