#ifndef BINARY_BUILDER
#define BINARY_BUILDER

#include "VirtualMachine.h"

namespace TucanScript::Binary {
	struct BinaryBuilder final {
		static Undef Decompose (const String& inFilePath, VM::Asm& asm_, VM::ReadOnlyData& roData);
		static Undef Build (const VM::ReadOnlyData& roData, const VM::Asm& asm_, const String& outFilePath);
	};
}

#endif