#include "Coroutines.h"

using namespace TucanScript;

ExternC {
	TucanAPI Undef SetCoroutineProps (ExC_Args) {
		auto scheduler = vm->GetScheduler ();
		scheduler->m_TaskMemoryProps.m_CallDepth = ExC_QWordArg (1u);
		scheduler->m_TaskMemoryProps.m_StackSize = ExC_QWordArg (0u);
	}

	TucanAPI Undef WaitForEachTask (ExC_Args) {
		vm->WaitForYield ();
	}

	TucanAPI Undef ResumeTask (ExC_Args) {
		vm->ResumeTask ((VM::HTask) ExC_NativePtrArg (0u));
	}

	TucanAPI Undef CloseTask (ExC_Args) {
		vm->CloseTask ((VM::HTask) ExC_NativePtrArg (0u));
	}
}