#include "Memory.h"

using namespace TucanScript;

ExternC {
#pragma region [Unsafe]
	TucanAPI Undef R_Alloc (ExC_Args) {
		stack->Push (VM::Val {
			.m_Type = VM::NATIVEPTR_T,
			.m_Data = VM::Word {
				.m_NativePtr = std::malloc (args->m_Memory[0].m_Data.m_U64)
			}
		});
	}

	TucanAPI Undef R_PtrToQWord (ExC_Args) {
		auto ptr = args->m_Memory[0];
		ptr.m_Type = VM::UINT64_T;
		stack->Push (ptr);
	}

	TucanAPI Undef R_QWordToPtr (ExC_Args) {
		auto ptr = args->m_Memory[0];
		ptr.m_Type = VM::NATIVEPTR_T;
		stack->Push (ptr);
	}

	TucanAPI Undef R_StrCat (ExC_Args) {
		auto a = args->m_Memory[0];
		auto b = args->m_Memory[1];
		StrOp (*stack, a, b, &std::strcat);
	}

	TucanAPI Undef R_StrCpy (ExC_Args) {
		auto a = args->m_Memory[0];
		auto b = args->m_Memory[1];
		StrOp (*stack, a, b, &std::strcpy);
	}

	TucanAPI Undef R_WriteWord (ExC_Args) {
		auto pBuffer = ExC_NativePtrArg (0);
		auto value = args->m_Memory[1];
		const Size szWord = VM::ValUtility::SizeMap.at(value.m_Type);
		std::memcpy (pBuffer, &value.m_Data, szWord);
	}

	TucanAPI Undef R_BufOffset (ExC_Args) {
		Undef* pBuffer = ExC_NativePtrArg (0);
		QWord szIncrement = ExC_QWordArg (1);
		stack->Push<Undef*, VM::NATIVEPTR_T> (pBuffer + szIncrement, &VM::Word::m_NativePtr);
	}
#pragma endregion

	TucanAPI Undef Merge (ExC_Args) {
		auto a = args->m_Memory[0];
		auto b = args->m_Memory[1];

		auto aSize = a.m_Data.m_ManagedPtr->m_Size;
		auto bSize = b.m_Data.m_ManagedPtr->m_Size;

		auto hAllocator = vm->GetAllocator ();

		auto merged = hAllocator->Alloc (aSize + bSize);

		std::memcpy (merged->m_Memory.m_hSpecBuf, a.m_Data.m_ManagedPtr->m_Memory.m_hSpecBuf, aSize);
		hAllocator->HandleReferences (a.m_Data.m_ManagedPtr);

		std::memcpy (merged->m_Memory.m_hSpecBuf + aSize, b.m_Data.m_ManagedPtr->m_Memory.m_hSpecBuf, bSize);
		hAllocator->HandleReferences (b.m_Data.m_ManagedPtr);

		stack->Push<VM::Managed*, VM::MANAGED_T> (merged, &VM::Word::m_ManagedPtr);
	}

	TucanAPI Undef GetRawBuf (ExC_Args) {
		stack->Push (args->m_Memory[0].m_Data.m_ManagedPtr->m_Memory.m_hRawBuf);
	}
}