#include "../Compiler.h"
#include "../BinaryBuilder.h"

using namespace TucanScript;

#define StackSize       512ULL
#define FixedMemorySize 512ULL
#define MaxCallDepth    1024ULL

#define nArg_StackSize       2
#define nArg_FixedMemorySize 3
#define nArg_MaxCallDepth    4

VM::VirtualMachine* pVM { nullptr };

static Undef FreeVM () { pVM->Free (); }

ProgramExitCode_t main (SInt32 nArgs, Sym* args[]) {
	if (nArgs <= 1) {
		LogErr ("Invalid binary file path argument!");
		return InvalidSignature;
	}

	VM::ReadOnlyData roData {};
	VM::Asm asm_ {};

	Binary::BinaryBuilder::Decompose (args[1], asm_, roData);

	auto pStaticDealloc = new VM::UnsafeDeallocator ();
	pStaticDealloc->PutReadOnlyData (roData);

	Size stackSize = StackSize;
	if (nArgs > nArg_StackSize && !TryParse (args[nArg_StackSize], stackSize)) {
		LogErr ("Invalid stack size format");
	}

	Size fixedMemorySize = FixedMemorySize;
	if (nArgs > nArg_FixedMemorySize && !TryParse (args[nArg_FixedMemorySize], fixedMemorySize)) {
		LogErr ("Invalid fixed memory size format");
	}

	Size callDepthLimit = MaxCallDepth;
	if (nArgs > nArg_MaxCallDepth && !TryParse (args[nArg_MaxCallDepth], callDepthLimit)) {
		LogErr ("Invalid call depth limit format");
	}

	pVM = new VM::VirtualMachine (
		stackSize,
		fixedMemorySize,
		callDepthLimit,
		asm_,
		pStaticDealloc);

	atexit (FreeVM);

    if (!pVM) {
        LogErr ("Virtual machine is already killed!");
        return _Fail;
    }

    pVM->Run();
    delete pVM;

	return Zero;
}