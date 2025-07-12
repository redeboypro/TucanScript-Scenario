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
	TucanAPI Undef R_Alloc (VM::VirtualMachine * vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef R_PtrToQWord (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef R_QWordToPtr (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef R_StrCat (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef R_StrCpy (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
#pragma endregion

	TucanAPI Undef Merge (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef GetRawBuf (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
	TucanAPI Undef SetCoroutineProps (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
}

#endif