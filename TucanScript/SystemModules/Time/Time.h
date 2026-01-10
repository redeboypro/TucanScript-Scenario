#ifndef MATH_H
#define MATH_H

#include <ctime>
#include "../../VirtualMachine.h"

using namespace TucanScript;

template <typename T>
Undef PushWord (VM::VirtualStack* stack, T value) {
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

	stack->Push (val);
}

ExternC {
	TucanAPI Undef GetProcessTime (ExC_Args);
	TucanAPI Undef GetSysTime (ExC_Args);
	TucanAPI Undef GetClocksPerSec (ExC_Args);
}

#endif