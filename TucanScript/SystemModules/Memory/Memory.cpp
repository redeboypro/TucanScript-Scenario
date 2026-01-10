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

	TucanAPI Undef R_MemCpy(ExC_Args) {
		const auto pFrom = static_cast<std::byte*>(args->m_Memory[0].m_Data.m_NativePtr);
		const auto pTo = static_cast<std::byte*>(args->m_Memory[1].m_Data.m_NativePtr);
		const auto uSz = args->m_Memory[0].m_Data.m_U64;
		std::memcpy (pTo, pFrom, uSz);
	}

	TucanAPI Undef R_Store(ExC_Args) {
		const auto pAlignedBuffer = static_cast<std::byte*>(args->m_Memory[0].m_Data.m_NativePtr);
		const auto uIndex = args->m_Memory[1].m_Data.m_U64;

		auto& [m_Type, m_Data] = args->m_Memory[2];
		const auto szWord = VM::ValUtility::SizeMap.at(m_Type);

		std::memcpy (pAlignedBuffer + uIndex * szWord, &m_Data, szWord);
	}

	TucanAPI Undef R_PtrToQWord (ExC_Args) {
		const auto& ptr = args->m_Memory[0];
		ptr.m_Type = VM::UINT64_T;
		stack->Push (ptr);
	}

	TucanAPI Undef R_QWordToPtr (ExC_Args) {
		const auto& ptr = args->m_Memory[0];
		ptr.m_Type = VM::NATIVEPTR_T;
		stack->Push (ptr);
	}

	TucanAPI Undef R_StrCat (ExC_Args) {
		const auto& a = args->m_Memory[0];
		const auto& b = args->m_Memory[1];
		StrOp (*stack, a, b, &std::strcat);
	}

	TucanAPI Undef R_StrCpy (ExC_Args) {
		const auto& a = args->m_Memory[0];
		const auto& b = args->m_Memory[1];
		StrOp (*stack, a, b, &std::strcpy);
	}

	TucanAPI Undef R_MemOffset (ExC_Args) {
		auto* pBuffer = VM::CBufferUtility::GetDirectMemory(args->m_Memory[0]);
		stack->Push<Undef*, VM::NATIVEPTR_T> (pBuffer + ExC_QWordArg (1), &VM::Word::m_NativePtr);
	}

	TucanAPI Undef R_Align (ExC_Args) {
		ExC_Return (Align (ExC_QWordArg (0), ExC_QWordArg (1)));
	}

	TucanAPI Undef R_AllocAligned (ExC_Args) {
		ExC_ReturnPtr (AllocAligned (ExC_QWordArg (0), ExC_QWordArg (1)));
	}

	TucanAPI Undef R_FreeAligned (ExC_Args) {
		FreeAligned (ExC_NativePtrArg(0));
	}

	TucanAPI Undef R_CpyAligned (ExC_Args) {
		CpyAligned (ExC_NativePtrArg (0),
			ExC_QWordArg (1),
			ExC_NativePtrArg(2),
			ExC_QWordArg (3),
			ExC_QWordArg (4));
	}

	TucanAPI Undef R_FCpy (ExC_Args) {
		FCpy (ExC_NativePtrArg (0),
		      ExC_QWordArg (1),
			  ExC_FloatArg (2));
	}

	TucanAPI Undef R_DCpy (ExC_Args) {
		DCpy (ExC_NativePtrArg (0),
			  ExC_QWordArg (1),
			  ExC_DoubleArg (2));
	}

	TucanAPI Undef R_I16Cpy(ExC_Args) {
		I16Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_Int16Arg(2)
		);
	}

	TucanAPI Undef R_I32Cpy(ExC_Args) {
		I32Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_Int32Arg(2)
		);
	}

	TucanAPI Undef R_I64Cpy(ExC_Args) {
		I64Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_Int64Arg(2)
		);
	}

	TucanAPI Undef R_U16Cpy(ExC_Args) {
		U16Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_WordArg(2)
		);
	}

	TucanAPI Undef R_U32Cpy(ExC_Args) {
		U32Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_DWordArg(2)
		);
	}

	TucanAPI Undef R_U64Cpy(ExC_Args) {
		U64Cpy(
			ExC_NativePtrArg(0),
			ExC_QWordArg(1),
			ExC_QWordArg(2)
		);
	}

    TucanAPI Undef R_NullPtr (ExC_Args) {
        stack->Push<Undef*, VM::NATIVEPTR_T> (nullptr, &VM::Word::m_NativePtr);
    }
#pragma endregion

	TucanAPI Undef Merge (ExC_Args) {
		auto& a = args->m_Memory[0];
		auto& b = args->m_Memory[1];

		const auto aSize = a.m_Data.m_ManagedPtr->m_Size;
		const auto bSize = b.m_Data.m_ManagedPtr->m_Size;

		const auto hAllocator = vm->GetAllocator ();

		const auto merged = hAllocator->Alloc (aSize + bSize);

		std::memcpy (merged->m_Memory.m_hSpecBuf, a.m_Data.m_ManagedPtr->m_Memory.m_hSpecBuf, aSize);
		hAllocator->HandleReferences (a.m_Data.m_ManagedPtr);

		std::memcpy (merged->m_Memory.m_hSpecBuf + aSize, b.m_Data.m_ManagedPtr->m_Memory.m_hSpecBuf, bSize);
		hAllocator->HandleReferences (b.m_Data.m_ManagedPtr);

		stack->Push<VM::Managed*, VM::MANAGED_T> (merged, &VM::Word::m_ManagedPtr);
	}

	TucanAPI Undef DirectMemory (ExC_Args) {
		stack->Push (args->m_Memory[0].m_Data.m_ManagedPtr->m_Memory.m_hRawBuf);
	}
}