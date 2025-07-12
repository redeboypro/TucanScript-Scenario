#include "Time.h"

ExternC {
	TucanAPI Undef GetProcessTime (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*) {
		PushWord (stack, std::clock ());
	}

	TucanAPI Undef GetSysTime (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*,const VM::ValMem*) {
		PushWord (stack, std::time (nullptr));
	}

	TucanAPI Undef GetClocksPerSec (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*,const VM::ValMem*) {
		PushWord (stack, CLOCKS_PER_SEC);
	}
}