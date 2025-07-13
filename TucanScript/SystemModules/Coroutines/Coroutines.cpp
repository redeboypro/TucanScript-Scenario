#include "Coroutines.h"

using namespace TucanScript;

TucanAPI Undef SetCoroutineProps (ExC_Args) {
	auto scheduler = vm->GetScheduler ();
	scheduler->m_TaskMemoryProps.m_CallDepth = args->m_Memory[1].m_Data.m_U64;
	scheduler->m_TaskMemoryProps.m_StackSize = args->m_Memory[0].m_Data.m_U64;
}

TucanAPI Undef WaitForEachTask (ExC_Args) {
	vm->WaitForYield ();
}

TucanAPI Undef ResumeTask (ExC_Args) {
	vm->ResumeTask ((VM::HTask) args->m_Memory[Zero].m_Data.m_ManagedPtr->m_Memory.m_hRawBuf);
}

TucanAPI Undef CloseTask (ExC_Args) {
	vm->CloseTask ((VM::HTask) args->m_Memory[Zero].m_Data.m_ManagedPtr->m_Memory.m_hRawBuf);
}