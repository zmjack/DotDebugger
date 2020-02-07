#include "pch.h"
#include "Udis86Wrapper.h"
#include <Windows.h>
#include "../udis86-1.7.2/Include/udis86.h"
#pragma comment(lib, "../udis86-1.7.2/Lib/x86/libudis86.lib")

typedef unsigned char byte;

extern "C" __declspec(dllexport) void __stdcall Disassemble(LPCVOID addr, SIZE_T size, LPCVOID buf)
{
	ud_t ud_obj;

	ud_init(&ud_obj);
	ud_set_mode(&ud_obj, 32);
	ud_set_input_buffer(&ud_obj, (uint8_t*)buf, size);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	int offset = 0;
	unsigned int length;
	while (length = ud_disassemble(&ud_obj))
	{
		printf("%p\t", addr);
		for (int i = 0; i < length; i++)
			printf("%02X ", ((byte*)buf)[offset + i]);
		printf("\t%s\n", ud_insn_asm(&ud_obj));

		addr = (byte*)addr + length;
		offset += length;
	}
}