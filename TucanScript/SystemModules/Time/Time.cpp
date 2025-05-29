#include "Time.h"

ExternC {
	TucanAPI Undef GetProcessTime (VM::VirtualMachine* vm, const VM::ValMem*) {
		PushWord (vm, std::clock ());
	}

	TucanAPI Undef GetSysTime (VM::VirtualMachine* vm, const VM::ValMem*) {
		PushWord (vm, std::time (nullptr));
	}

	TucanAPI Undef GetClocksPerSec (VM::VirtualMachine* vm, const VM::ValMem*) {
		PushWord (vm, CLOCKS_PER_SEC);
	}
}