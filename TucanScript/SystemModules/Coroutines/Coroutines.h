#ifndef COROUTINES_H
#define COROUTINES_H

#include "../../VirtualMachine.h"

using namespace TucanScript;

ExternC {
	TucanAPI Undef SetCoroutineProps (ExC_Args);
	TucanAPI Undef WaitForEachTask (ExC_Args);
	TucanAPI Undef ResumeTask (ExC_Args);
	TucanAPI Undef CloseTask (ExC_Args);
}

#endif