#ifndef MEMORY_H
#define MEMORY_H

#include "../../VirtualMachine.h"

using namespace TucanScript;

inline Undef StrOp (VM::VirtualStack& stack, 
					const VM::Val& a, const VM::Val& b, 
					Sym* (*op)(Sym*, const Sym*)) {
	auto* aStr = static_cast<Sym *>(a.m_Data.m_NativePtr);
	const auto* bStr = static_cast<Sym *>(b.m_Data.m_NativePtr);
	stack.Push <Undef*, VM::NATIVEPTR_T> (op (aStr, bStr), &VM::Word::m_NativePtr);
}

ExternC {
#pragma region [Raw]
	TucanAPI Undef R_Alloc (ExC_Args);
	TucanAPI Undef R_MemCpy (ExC_Args);
	TucanAPI Undef R_Store (ExC_Args);
	TucanAPI Undef R_PtrToQWord (ExC_Args);
	TucanAPI Undef R_QWordToPtr (ExC_Args);
	TucanAPI Undef R_StrCat (ExC_Args);
	TucanAPI Undef R_StrCpy (ExC_Args);
	TucanAPI Undef R_MemOffset (ExC_Args);
	TucanAPI Undef R_Align (ExC_Args);
	TucanAPI Undef R_AllocAligned (ExC_Args);
	TucanAPI Undef R_FreeAligned (ExC_Args);
	TucanAPI Undef R_CpyAligned (ExC_Args);
	TucanAPI Undef R_FCpy (ExC_Args);
	TucanAPI Undef R_DCpy (ExC_Args);
	TucanAPI Undef R_I16Cpy (ExC_Args);
	TucanAPI Undef R_I32Cpy (ExC_Args);
	TucanAPI Undef R_I64Cpy (ExC_Args);
	TucanAPI Undef R_U16Cpy (ExC_Args);
	TucanAPI Undef R_U32Cpy (ExC_Args);
	TucanAPI Undef R_U64Cpy (ExC_Args);
    TucanAPI Undef R_NullPtr (ExC_Args);

#pragma endregion

	TucanAPI Undef Merge (ExC_Args);
	TucanAPI Undef DirectMemory (ExC_Args);
}

#endif