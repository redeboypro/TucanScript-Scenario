#ifndef MATH_H
#define MATH_H

#include "time.h"
#include "../../VirtualMachine.h"

using namespace TucanScript;

template <typename T>
Undef PushWord (VM::VirtualMachine* vm, T value) {
	VM::Val val {};
	std::memcpy (&val.m_Data, &value, sizeof (T));

	if constexpr (sizeof (T) == sizeof (SInt32)) {
		val.m_Type = VM::INT32_T;
	}
	else if constexpr (sizeof (T) == sizeof (SInt64)) {
		val.m_Type = VM::INT64_T;
	}
	else {
		val.m_Type = VM::UINT64_T;
	}

	vm->GetStack ()->Push (val);
}

ExternC {
	TucanAPI Undef GetProcessTime (VM::VirtualMachine* vm, const VM::ValMem*);
	TucanAPI Undef GetSysTime (VM::VirtualMachine* vm, const VM::ValMem*);
	TucanAPI Undef GetClocksPerSec (VM::VirtualMachine* vm, const VM::ValMem*);
}

#endif