#include "Time.h"

ExternC {
	TucanAPI Undef GetProcessTime (ExC_Args) {
		PushWord (stack, std::clock ());
	}

	TucanAPI Undef GetSysTime (ExC_Args) {
		PushWord (stack, std::time (nullptr));
	}

	TucanAPI Undef GetClocksPerSec (ExC_Args) {
		PushWord (stack, CLOCKS_PER_SEC);
	}
}