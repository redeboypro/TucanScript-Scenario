#include "../Compiler.h"
#include "../BinaryBuilder.h"

using namespace TucanScript;

#define ExtSeparator "."
#define BinaryExt    ".tbin"

#define StackSize       512ULL
#define FixedMemorySize 512ULL
#define MaxCallDepth    1024

#define nArg_StackSize       2
#define nArg_FixedMemorySize 3
#define nArg_MaxCallDepth    4

ProgramExitCode_t main (SInt32 nArgs, Sym* args[]) {
	if (nArgs <= 1) {
		LogErr ("Invalid binary file path argument!");
		return InvalidSignature;
	}

	TucanScript::VM::ReadOnlyData roData {};
	TucanScript::VM::Asm asm_ {};

	TucanScript::Binary::BinaryBuilder::Decompose (args[1], asm_, roData);

	auto staticDealloc = new TucanScript::VM::UnsafeDeallocator ();
	staticDealloc->PutReadOnlyData (roData);

	Size stackSize = StackSize;
	if (nArgs > nArg_StackSize && !TryParse (args[nArg_StackSize], stackSize)) {
		LogErr ("Invalid stack size format");
	}

	Size fixedMemorySize = FixedMemorySize;
	if (nArgs > nArg_FixedMemorySize && !TryParse (args[nArg_FixedMemorySize], fixedMemorySize)) {
		LogErr ("Invalid fixed memory size format");
	}

	SInt32 callDepthLimit = MaxCallDepth;
	if (nArgs > nArg_MaxCallDepth && !TryParse (args[nArg_MaxCallDepth], callDepthLimit)) {
		LogErr ("Invalid call depth limit format");
	}

	auto vm = new TucanScript::VM::VirtualMachine (
		stackSize,
		fixedMemorySize,
		callDepthLimit,
		std::move (asm_),
		staticDealloc);

	vm->Run ();

	delete vm;
	return Zero;
}