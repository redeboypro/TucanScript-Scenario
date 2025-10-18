#ifndef MEMORY_H
#define MEMORY_H

#include "../../VirtualMachine.h"

using namespace TucanScript;

inline Undef StrOp (VM::VirtualStack& stack, 
					const VM::Val& a, const VM::Val& b, 
					Sym* (*op)(Sym*, const Sym*)) {
	auto* aStr = (Sym*)(a.m_Data.m_NativePtr);
	auto* bStr = (Sym*)(b.m_Data.m_NativePtr);
	stack.Push <Undef*, VM::NATIVEPTR_T> (op (aStr, bStr), &VM::Word::m_NativePtr);
}

ExternC {
#pragma region [Raw]
	TucanAPI Undef R_Alloc (ExC_Args);
	TucanAPI Undef R_PtrToQWord (ExC_Args);
	TucanAPI Undef R_QWordToPtr (ExC_Args);
	TucanAPI Undef R_StrCat (ExC_Args);
	TucanAPI Undef R_StrCpy (ExC_Args);
	TucanAPI Undef R_WriteWord (ExC_Args);
	TucanAPI Undef R_BufOffset (ExC_Args);
#pragma endregion

	TucanAPI Undef Merge (ExC_Args);
	TucanAPI Undef GetRawBuf (ExC_Args);
}

#endif