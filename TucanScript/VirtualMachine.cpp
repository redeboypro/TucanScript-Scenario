#include "VirtualMachine.h"

using namespace TucanScript;

TucanScript::VM::UnsafeDeallocator::UnsafeDeallocator () : m_Handles (nullptr), m_NumHandles (NULL) {}

TucanScript::VM::UnsafeDeallocator::~UnsafeDeallocator () {
	Free ();
}

UInt64 TucanScript::VM::UnsafeDeallocator::GetSizeInBytes () const {
	return m_NumHandles * sizeof (UnsafeMemory);
}

Undef* TucanScript::VM::UnsafeDeallocator::operator[](UInt64 index) {
	return m_Handles[index].m_Handle;
}

Undef TucanScript::VM::UnsafeDeallocator::PutHandle (const UnsafeMemory& handle) {
	auto* newHandles = std::realloc (m_Handles, NextWord (m_NumHandles) * sizeof (UnsafeMemory));

	if (!newHandles) {
		LogInstErr (nameof (CMEMALLOC), "Failed to allocate memory for native handles!");
		return;
	}

	m_Handles = (UnsafeMemory*) newHandles;
	m_Handles[m_NumHandles++] = handle;
}

Undef TucanScript::VM::UnsafeDeallocator::PutReadOnlyData (const ReadOnlyData& roData) {
	for (Size iLiteral = Zero; iLiteral < roData.m_Size; iLiteral++) {
		PutHandle (UnsafeMemory {
			.m_Type   = STATIC_MEMORY,
			.m_Handle = roData.m_Memory[iLiteral]
		});
	}
}

Undef TucanScript::VM::UnsafeDeallocator::Free () {
	if (!m_Handles) {
		return;
	}

	for (UInt64 iHandle = Zero; iHandle < m_NumHandles; ++iHandle) {
		const auto& handle = m_Handles[iHandle];
		if (handle.m_Type == STATIC_MEMORY) {
			std::free (handle.m_Handle);
		}
		else {
			FreeLib (handle.m_Handle);
		}
	}

	std::free (m_Handles);

	m_Handles    = nullptr;
	m_NumHandles = Zero;
}

TucanScript::VM::VMStack::VMStack (Size size) : m_End (Zero), m_Size (size) {
	m_Data = new Val[size];
}

TucanScript::VM::VMStack::~VMStack () {
	delete[] m_Data;
}

Undef TucanScript::VM::VMStack::Push (Val value) {
	m_Data[m_End] = value;
	m_End++;
}

Undef TucanScript::VM::VMStack::Push (Boolean value) {
	Push (static_cast<SInt32>(value));
}

Undef TucanScript::VM::VMStack::Push (SInt8 value) {
	Push <SInt8, CHAR_T> (value, &Word::m_C);
}

Undef TucanScript::VM::VMStack::Push (UInt8 value) {
	Push <UInt8, BYTE_T> (value, &Word::m_UC);
}

Undef TucanScript::VM::VMStack::Push (UInt16 value) {
	Push <UInt16, UINT16_T> (value, &Word::m_U16);
}

Undef TucanScript::VM::VMStack::Push (UInt32 value) {
	Push <UInt32, UINT32_T> (value, &Word::m_U32);
}

Undef TucanScript::VM::VMStack::Push (UInt64 value) {
	Push <UInt64, UINT64_T> (value, &Word::m_U64);
}

Undef TucanScript::VM::VMStack::Push (SInt16 value) {
	Push <SInt16, INT16_T> (value, &Word::m_I16);
}

Undef TucanScript::VM::VMStack::Push (SInt32 value) {
	Push <SInt32, INT32_T> (value, &Word::m_I32);
}

Undef TucanScript::VM::VMStack::Push (SInt64 value) {
	Push <SInt64, INT64_T> (value, &Word::m_I64);
}

Undef TucanScript::VM::VMStack::Push (Dec32 value) {
	Push <Dec32, FLOAT32_T> (value, &Word::m_F32);
}

Undef TucanScript::VM::VMStack::Push (Dec64 value) {
	Push <Dec64, FLOAT64_T> (value, &Word::m_F64);
}

VM::Val TucanScript::VM::VMStack::Pop () {
	m_End--;
	return m_Data[m_End];
}

TucanScript::VM::VMAllocator::VMAllocator () : m_Begin (nullptr), m_End (nullptr) {}

const VM::Managed& TucanScript::VM::VMAllocator::Begin () {
	return *m_Begin;
}

const VM::Managed& TucanScript::VM::VMAllocator::End () {
	return *m_End;
}

VM::Managed* TucanScript::VM::VMAllocator::Alloc (UInt64 size) {
	auto* allocated = new Managed {
		.m_Memory = (Val*) std::malloc (size * sizeof (Val)),
		.m_Size = size,
		.m_Next = nullptr,
		.m_Previous = m_End
	};

	if (m_End) {
		m_End->m_Next = allocated;
	}

	m_End = allocated;

	if (!m_Begin) {
		m_Begin = allocated;
	}

	m_NumBlocks++;

	return allocated;
}

Undef TucanScript::VM::VMAllocator::Free (Managed* ptr, Boolean removeReferences) {
	auto* prevPtr = ptr->m_Previous;
	auto* nextPtr = ptr->m_Next;

	if (prevPtr) {
		prevPtr->m_Next = nextPtr;
	}
	else {
		m_Begin = nextPtr;
	}

	if (nextPtr) {
		nextPtr->m_Previous = prevPtr;
	}
	else {
		m_End = prevPtr;
	}

	for (SInt32 iValue = Zero; removeReferences && iValue < ptr->m_Size; ++iValue) {
		auto& value = ptr->m_Memory[iValue];
		if (value.m_Type == MANAGED_T) {
			RemoveRef (value.m_Data.m_ManagedPtr);
		}
	}

	std::free (ptr->m_Memory);
	m_NumBlocks--;

	delete ptr;
}

Undef TucanScript::VM::VMAllocator::FreeAll () {
	while (m_NumBlocks > Zero && m_Begin) {
		Free (m_Begin, false);
	}
}

VM::Val TucanScript::VM::VirtualMachine::Unpack (JmpMemory& frame, const Val& value) const {
	auto* memory = GetMemoryAtAddress (frame, value, nullptr);
	return memory?*memory:value;
}

VM::Val TucanScript::VM::VirtualMachine::PopUnpack (VMStack& stack, JmpMemory& frame) {
	return Unpack (frame, stack.Pop ());
}

#define InvalidInstPosValType "Invalid instruction position value type!"
Undef TucanScript::VM::VirtualMachine::Jmp (SInt64& iInst, Instruction& instruction) {
	auto instructionValue = instruction.m_Val;
	if (instructionValue.m_Type Is INT64_T) {
		iInst = PrevWord (instructionValue.m_Data.m_I64);
	}
	else {
		LogInstErr (nameof (JMP), InvalidInstPosValType);
		Free ();
		return;
	}
}

Undef TucanScript::VM::VirtualMachine::FreeManagedMemory (Val* memory) {
	if (memory->m_Type == MANAGED_T) {
		m_Allocator.Free (memory->m_Data.m_ManagedPtr);
	}
	else if (memory->m_Type == NATIVEPTR_T) {
		std::free (memory->m_Data.m_NativePtr);
	}
	else {
		memory->m_Data.m_U64 = Zero;
	}
}

Undef TucanScript::VM::VirtualMachine::MemCopy (VMStack& stack, MemCpyFrameArgs frameArgs, const Val& dest, const Val& src, Boolean pushBack) {
	auto srcUnpacked = Unpack (*frameArgs.m_SrcFrame, src);

	auto* destUnpackTry = GetMemoryAtAddress (*frameArgs.m_DestFrame, dest, nullptr);
	auto& destUnpacked = destUnpackTry?*destUnpackTry:dest;

	if (destUnpacked.m_Type Is NATIVEPTR_T) {
		std::memcpy (destUnpacked.m_Data.m_NativePtr,
					 &srcUnpacked.m_Data, 
					 ValUtility::SizeMap.at (srcUnpacked.m_Type));
		if (pushBack) {
			stack.Push (destUnpacked);
		}
		return;
	}

	if (destUnpacked.m_Type Is MANAGED_T) {
		m_Allocator.RemoveRef (destUnpacked.m_Data.m_ManagedPtr);
	}

	destUnpacked.m_Type = srcUnpacked.m_Type;
	destUnpacked.m_Data = srcUnpacked.m_Data;

	if (destUnpacked.m_Type Is MANAGED_T) {
		destUnpacked.m_Data.m_ManagedPtr->m_RefCount++;
	}

	if (pushBack) {
		stack.Push (dest);
	}
}

VM::Val* TucanScript::VM::VirtualMachine::GetMemoryAtAddress (JmpMemory& frame, const Val& src, Val* defaultValue) const {
	if (src.m_Type == RADDRESS_T) {
		return GetMemoryAtRAddress (src.m_Data.m_I32);
	}
	else if (src.m_Type == LRADDRESS_T) {
		return GetMemoryAtLRAddress (frame, src.m_Data.m_I32);
	}
	return defaultValue;
}

Undef TucanScript::VM::VirtualMachine::StrOp (VMStack& stack, const Val& a, const Val& b, Sym* (*op)(Sym*, const Sym*)) {
	auto* aStr = (Sym*)(a.m_Data.m_NativePtr);
	auto* bStr = (Sym*)(b.m_Data.m_NativePtr);
	stack.Push <Undef*, NATIVEPTR_T> (op (aStr, bStr), &Word::m_NativePtr);
}

Sym* TucanScript::VM::VirtualMachine::GetCStr (const Managed* managedMemory) {
	const Size strLength = managedMemory->m_Size;

	if (strLength == Zero) {
		return nullptr;
	}

	auto* cStr = (Sym*) std::malloc (NextWord (strLength));
	if (!cStr) {
		return nullptr;
	}

	cStr[strLength] = Zero;
	for (SInt32 iSym = Zero; iSym < strLength; iSym++) {
		cStr[iSym] = managedMemory->m_Memory[iSym].m_Data
		#if CHAR_MIN < Zero
			.m_C;
		#else
			.m_UC;
		#endif
	}
	return cStr;
}

Undef TucanScript::VM::VirtualMachine::AllocStr (VMStack& stack, Sym* buffer, Size size) {
	auto* memory = m_Allocator.Alloc (size);

	for (Size iSym = Zero; iSym < size; iSym++) {
		memory->m_Memory[iSym] = Val {
		#if CHAR_MIN < Zero
			.m_Type = CHAR_T,
			.m_Data = Word {
				.m_C = buffer[iSym]
				}
		#else
			.m_Type = BYTE_T,
			.m_Data = Word {
				.m_UC = buffer[iSym]
				}
		#endif
		};
	}

	stack.Push (Val {
		.m_Type = MANAGED_T,
		.m_Data =
			Word {
				.m_ManagedPtr = memory
			}
	});
}

#define ApplyOp(AVAL, BVAL, OP)                                     \
switch ((AVAL).m_Type) {                                            \
	case CHAR_T:                                                    \
	stack.Push ((AVAL).m_Data.m_C OP (BVAL).m_Data.m_C);            \
	break;                                                          \
	case BYTE_T:                                                    \
	stack.Push ((AVAL).m_Data.m_UC OP (BVAL).m_Data.m_UC);          \
	break;                                                          \
	case UINT16_T:                                                  \
	stack.Push ((AVAL).m_Data.m_U16 OP (BVAL).m_Data.m_U16);        \
	break;                                                          \
	case UINT32_T:                                                  \
	stack.Push ((AVAL).m_Data.m_U32 OP (BVAL).m_Data.m_U32);        \
	break;                                                          \
	case UINT64_T:                                                  \
	stack.Push ((AVAL).m_Data.m_U64 OP (BVAL).m_Data.m_U64);        \
	break;                                                          \
	case INT16_T:                                                   \
	stack.Push ((AVAL).m_Data.m_I16 OP (BVAL).m_Data.m_I16);        \
	break;                                                          \
	case INT32_T:                                                   \
	stack.Push ((AVAL).m_Data.m_I32 OP (BVAL).m_Data.m_I32);        \
	break;                                                          \
	case INT64_T:                                                   \
	stack.Push ((AVAL).m_Data.m_I64 OP (BVAL).m_Data.m_I64);        \
	break;                                                          \
	case FLOAT32_T:                                                 \
	stack.Push ((AVAL).m_Data.m_F32 OP (BVAL).m_Data.m_F32);        \
	break;                                                          \
	case FLOAT64_T:                                                 \
	stack.Push ((AVAL).m_Data.m_F64 OP (BVAL).m_Data.m_F64);        \
	break;                                                          \
}

#define _Success 0x1ll
#define _Fail    -(_Success)
#define _Exit    _Fail

#define InvalidStackPopVal "Invalid stack popped value type!"
TucanScript::SInt32 TucanScript::VM::VirtualMachine::HandleInstr (SInt64& qInstr, VMStack& stack, JmpMemory& frame) {
	auto& instruction = m_Asm.m_Memory[qInstr];
	switch (instruction.m_Op) {
		case HALT: {
			return _Exit;
		}
		case PUSH: {
			stack.Push (instruction.m_Val);
			break;
		}
		case POP: {
			stack.Pop ();
			break;
		}
		case JMP: {
			Jmp (qInstr, instruction);
			break;
		}
		case JMPC: {
			auto instructionValue = instruction.m_Val;
			if (instructionValue.m_Type Is INT64_T) {
				if (!IsTrue (PopUnpack (stack, frame))) {
					qInstr = PrevWord (instructionValue.m_Data.m_I64);
				}
			}
			else {
				LogInstErr (nameof (JMPC), InvalidInstPosValType);
				Free ();
				return _Fail;
			}
			break;
		}
		case JMPR: {
			DoRecordJump (qInstr, qInstr, stack, &frame);
			break;
		}
		case CALLASYNC: {
			lpTask asyncTask = m_TaskPool.Run (_Exit);

			MemCpyFrameArgs frameArgs;
			frameArgs.m_SrcFrame  = &frame;
			frameArgs.m_DestFrame = &asyncTask->m_Frame;

			DoRecordJump (asyncTask->m_qInstr, qInstr, stack, frameArgs);
			asyncTask->m_qInstr++;
			break;
		}
		case RETURN: {
			SInt32 numArgs = PopUnpack (stack, frame).m_Data.m_I32;
			SInt32 resultAddress = InvalidID;
			if (numArgs > Zero) {
				auto result = stack.Pop ();
				if (result.m_Type Is LRADDRESS_T) {
					resultAddress = result.m_Data.m_I32;
				}
				stack.Push (Unpack (frame, result));
			}
			auto& callMemory = GetLastCall (frame).m_Memory;
			for (SInt32 iValue = Zero; iValue < callMemory.m_Size; iValue++) {
				auto& value = callMemory.m_Memory[iValue];
				if (value.m_Type Is MANAGED_T) {
					value.m_Data.m_ManagedPtr->m_RefCount--;
					if (iValue != resultAddress) {
						m_Allocator.HandleReferences (value.m_Data.m_ManagedPtr);
					}
				}
			}
			delete[] callMemory.m_Memory;
			callMemory.m_Memory = nullptr;
			frame.m_Depth--;
			if ((qInstr = PrevWord (GetLastCall (frame).m_Address)) == _Exit) {
				return qInstr;
			}
			break;
		}
		case MEMSIZE: {
			auto src = PopUnpack (stack, frame);
			if (src.m_Type Is MANAGED_T) {
				stack.Push (GetMemorySize (src));
			}
			else if (src.m_Type Is NATIVEPTR_T) {
				stack.Push ((Size)std::strlen (reinterpret_cast <Sym*>(src.m_Data.m_NativePtr)));
			}
			else {
				LogInstErr (nameof (MEMSIZE), InvalidStackPopVal);
				Free ();
				return _Fail;
			}
			break;
		}
		case MEMLOAD: {
			auto id = PopUnpack (stack, frame);
			auto src = PopUnpack (stack, frame);

			if (src.m_Type Is MANAGED_T) {
				auto* managedMemory = src.m_Data.m_ManagedPtr;
				stack.Push (managedMemory->m_Memory[id.m_Data.m_U64]);
				m_Allocator.HandleReferences (managedMemory);
			}
			else {
				LogInstErr (nameof (MEMLOAD), InvalidStackPopVal);
				Free ();
				return _Fail;
			}
			break;
		}
		case MEMSTORE: {
			auto src = stack.Pop ();
			auto id = PopUnpack (stack, frame);
			auto dest = stack.Pop ();

			if (auto* memory = GetMemoryAtAddress (frame, dest, nullptr)) {
				if (memory->m_Type Is MANAGED_T) {
					MemCopy (stack, &frame, memory->m_Data.m_ManagedPtr->m_Memory[id.m_Data.m_U64], src, true);
				}
			}
			else {
				LogInstErr (nameof (MEMSTORE), InvalidStackPopVal);
				Free ();
				return _Fail;
			}
			break;
		}
		case STRALLOC: {
			auto* cStr = (Sym*)(*m_GlobalDeallocator)[instruction.m_Val.m_Data.m_U64];
			auto strLength = std::strlen (cStr);
			AllocStr (stack, cStr, strLength);
			break;
		}
		case SEQUENCEALLOC: {
			auto size = instruction.m_Val.m_Data.m_U64;
			auto* memory = m_Allocator.Alloc (size);

			for (SInt32 iElement = PrevWord (size); iElement >= Zero; iElement--) {
				auto poppedValue = PopUnpack (stack, frame);
				if (poppedValue.m_Type Is MANAGED_T) {
					poppedValue.m_Data.m_ManagedPtr->m_RefCount++;
				}
				memory->m_Memory[iElement] = poppedValue;
			}

			stack.Push (Val {
				.m_Type = MANAGED_T,
				.m_Data =
					Word {
						.m_ManagedPtr = memory
					}
						  });
			break;
		}
		case CMEMALLOC: {
			stack.Push (Val {
				.m_Type = NATIVEPTR_T,
				.m_Data =
					Word {
							.m_NativePtr = std::malloc (PopUnpack (stack, frame).m_Data.m_U64)
					}
						  });
			break;
		}
		case MEMALLOC: {
			stack.Push (Val {
				.m_Type = MANAGED_T,
				.m_Data =
					Word {
							.m_ManagedPtr = m_Allocator.Alloc (PopUnpack (stack, frame).m_Data.m_U64)
					}
						  });
			break;
		}
		case MEMAPPEND: {
			auto newElement = PopUnpack (stack, frame);
			auto memoryHolder = PopUnpack (stack, frame);

			if (memoryHolder.m_Type Is MANAGED_T) {
				auto* managedMemory = memoryHolder.m_Data.m_ManagedPtr;

				const UInt64 newSize = (NextWord (managedMemory->m_Size)) * sizeof (Val);
				auto newMemory = std::realloc (managedMemory->m_Memory, newSize);

				if (newMemory) {
					managedMemory->m_Memory = (Val*)newMemory;
					managedMemory->m_Memory[managedMemory->m_Size++] = newElement;
				}
				else {
					LogInstErr (nameof (MEMAPPEND), "Failed to realloc!");
					Free ();
					return _Fail;
				}
			}
			break;
		}
		case MEMDEALLOC: {
			auto poppedValue = stack.Pop ();

			if (poppedValue.m_Type Is RADDRESS_T) {
				FreeManagedMemory (GetMemoryAtRAddress (poppedValue.m_Data.m_I32));
				break;
			}
			else if (poppedValue.m_Type Is LRADDRESS_T) {
				FreeManagedMemory (GetMemoryAtLRAddress (frame, poppedValue.m_Data.m_I32));
				break;
			}

			LogInstErr (nameof (MEMDEALLOC), InvalidStackPopVal);
			Free ();
			return _Fail;
		}
		case MEMCPY: {
			auto src = stack.Pop ();
			auto dest = stack.Pop ();
			MemCopy (stack, &frame, dest, src, true);
			break;
		}
		case TOC: {
			Cast <SInt8> (frame, CHAR_T, &Word::m_C);
			break;
		}
		case TOUC: {
			Cast <UInt8> (frame, BYTE_T, &Word::m_UC);
			break;
		}
		case TOU16: {
			Cast <UInt16> (frame, UINT16_T, &Word::m_U16);
			break;
		}
		case TOU32: {
			Cast <UInt32> (frame, UINT32_T, &Word::m_U32);
			break;
		}
		case TOU64: {
			Cast <UInt64> (frame, UINT64_T, &Word::m_U64);
			break;
		}
		case TOI16: {
			Cast <SInt16> (frame, INT16_T, &Word::m_I16);
			break;
		}
		case TOI32: {
			Cast <SInt32> (frame, INT32_T, &Word::m_I32);
			break;
		}
		case TOI64: {
			Cast <SInt64> (frame, INT64_T, &Word::m_I64);
			break;
		}
		case TOF32: {
			Cast <Dec32> (frame, FLOAT32_T, &Word::m_F32);
			break;
		}
		case TOF64: {
			Cast <Dec64> (frame, FLOAT64_T, &Word::m_F64);
			break;
		}
		case ADD: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, +);
			break;
		}
		case SUB: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, -);
			break;
		}
		case MUL: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, *);
			break;
		}
		case DIV: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, / );
			break;
		}
		case CMPE: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, == );
			break;
		}
		case CMPNE: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, != );
			break;
		}
		case CMPG: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, > );
			break;
		}
		case CMPL: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, < );
			break;
		}
		case CMPGE: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, >= );
			break;
		}
		case CMPLE: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, <= );
			break;
		}
		case AND: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, &&);
			break;
		}
		case OR: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			ApplyOp (a, b, || );
			break;
		}
		case PRINT: {
			Print (PopUnpack (stack, frame));
			break;
		}
		case SCAN: {
			auto bufferSize = PopUnpack (stack, frame);
			if (!TryCast <UInt64> (bufferSize, &Word::m_U64)) {
				bufferSize.m_Data.m_U64 = 1024ULL; //Default buffer length
			}
			Size bufferLength = bufferSize.m_Data.m_U64;
			Sym* buffer = (Sym*)std::malloc (bufferLength);
			std::cin.getline (buffer, bufferLength);
			Size strLength = std::strlen (buffer);
			AllocStr (stack, buffer, strLength);
			std::free (buffer);
			break;
		}
	#if _WIN32
		case PTR2DWORD: {
			auto ptr = PopUnpack (stack, frame);
			{
				ptr.m_Type = UINT32_T;
				ptr.m_Data.m_U32 = reinterpret_cast<UInt32> (ptr.m_Data.m_NativePtr);
			}
			stack.Push (ptr);
			break;
		}
	 #endif
		case PTR2QWORD: {
			auto ptr = PopUnpack (stack, frame);
			{
				ptr.m_Type = UINT64_T;
				ptr.m_Data.m_U64 = reinterpret_cast<UInt64> (ptr.m_Data.m_NativePtr);
			}
			stack.Push (ptr);
			break;
		}
	#if _WIN32
		case DWORD2PTR: {
			auto ptr = PopUnpack (stack, frame);
			{
				ptr.m_Type = NATIVEPTR_T;
				ptr.m_Data.m_NativePtr = reinterpret_cast<Undef*> (ptr.m_Data.m_U32);
			}
			stack.Push (ptr);
			break;
		}
	#endif
		case QWORD2PTR: {
			auto ptr = PopUnpack (stack, frame);
			{
				ptr.m_Type = NATIVEPTR_T;
				ptr.m_Data.m_NativePtr = reinterpret_cast<Undef*> (ptr.m_Data.m_U64);
			}
			stack.Push (ptr);
			break;
		}
		case WRAP: {
			auto* memory = GetMemoryAtAddress (frame, stack.Pop (), nullptr);
			if (!memory) {
				LogInstErr (nameof (WRAP), "Invalid destination address!");
				Free ();
				return _Fail;
			}
			stack.Push <Undef*, NATIVEPTR_T> (&memory->m_Data, &Word::m_NativePtr);
			break;
		}
		case CONCAT: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);

			auto aLength = GetMemorySize (a);
			auto bLength = GetMemorySize (b);

			auto managedMemory = m_Allocator.Alloc (aLength + bLength);

			for (UInt64 iCharA = Zero; iCharA < aLength; iCharA++) {
				managedMemory->m_Memory[iCharA] = a.m_Data.m_ManagedPtr->m_Memory[iCharA];
			}
			m_Allocator.HandleReferences (a.m_Data.m_ManagedPtr);

			for (UInt64 iCharB = Zero; iCharB < bLength; iCharB++) {
				managedMemory->m_Memory[iCharB + aLength] = b.m_Data.m_ManagedPtr->m_Memory[iCharB];
			}
			m_Allocator.HandleReferences (b.m_Data.m_ManagedPtr);

			stack.Push <Managed*, MANAGED_T> (managedMemory, &Word::m_ManagedPtr);
			break;
		}
		case STRCAT: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			StrOp (stack, a, b, &std::strcat);
			break;
		}
		case STRCPY: {
			auto b = PopUnpack (stack, frame);
			auto a = PopUnpack (stack, frame);
			StrOp (stack, a, b, &std::strcpy);
			break;
		}
		case LOADLIB: {
			auto* hName = PopUnpack (stack, frame).m_Data.m_ManagedPtr;
			auto* cStrName = GetCStr (hName);

			Undef* hLib = GetModule (cStrName);
			if (!hLib) {
				hLib = LoadLib (cStrName);

				if (!hLib) {
					LogInstErr (nameof (LOADLIB), "Failed to load library: " << cStrName);
					std::free (cStrName);
					Free ();
					return _Fail;
				}

				m_GlobalDeallocator->PutHandle (
					UnsafeMemory {
						.m_Type = NATIVE_LIBRARY,
						.m_Handle = hLib,
					}
				);
			}
			std::free (cStrName);
			m_Allocator.HandleReferences (hName);

			stack.Push <Undef*, NATIVEPTR_T> (hLib, &Word::m_NativePtr);
			break;
		}
		case LOADSYM: {
			auto* hName = PopUnpack (stack, frame).m_Data.m_ManagedPtr;
			auto* hLib = PopUnpack (stack, frame).m_Data.m_NativePtr;

			auto* cStrName = GetCStr (hName);
			auto* hSym = ProcAddr (hLib, cStrName);
			std::free (cStrName);

			if (!hSym) {
				LogInstErr (nameof (LOADSYM), "Failed to load symbol: " << cStrName);
				Free ();
				return _Fail;
			}

			m_Allocator.HandleReferences (hName);

			stack.Push <Undef*, NATIVEPTR_T> (hSym, &Word::m_NativePtr);
			break;
		}
		case DOEXCALL: {
			SInt32 nArgs = instruction.m_Val.m_Data.m_I32;
			ValMem args {
				.m_Memory = new Val[nArgs],
				.m_Size = static_cast<Size> (nArgs)
			};

			auto hSym = PopUnpack (stack, frame).m_Data.m_NativePtr;
			if (!hSym) {
				LogInstErr (nameof (DOEXCALL), "Null function pointer!");
				Free ();
				return _Fail;
			}

			for (SInt32 iArg = PrevWord (nArgs); iArg >= Zero; iArg--) {
				args.m_Memory[iArg] = PopUnpack (stack, frame);
			}

			((ExCall_t) hSym) (this, &args);
			delete[] args.m_Memory;
			break;
		}
		case SETTASKPROPS: {
			m_TaskPool.m_TaskMemoryProps.m_CallDepth = PopUnpack (stack, frame).m_Data.m_U64;
			m_TaskPool.m_TaskMemoryProps.m_StackSize = PopUnpack (stack, frame).m_Data.m_U64;
		}
	}

	return _Success;
}

Undef TucanScript::VM::VirtualMachine::DoRecordJump (SInt64& qContextInstr, SInt64& qTargetInstr, VMStack& stack, const MemCpyFrameArgs& frameArgs) {
	GetLastCall (*frameArgs.m_DestFrame).m_Address = NextWord (qContextInstr);
	frameArgs.m_DestFrame->m_Depth++;

	SInt32 callMemorySize = PopUnpack (stack, *frameArgs.m_SrcFrame).m_Data.m_I32;
	const SInt32 numArgs = PopUnpack (stack, *frameArgs.m_SrcFrame).m_Data.m_I32;

	auto& callMemory = GetLastCall (*frameArgs.m_DestFrame).m_Memory;
	callMemory.m_Memory = new Val[callMemorySize];
	callMemory.m_Size = callMemorySize;

	for (SInt32 iArg = PrevWord (numArgs); iArg >= Zero; iArg--) {
		auto arg = ValUtility::_DWORD_signed_raw (&iArg, false);
		arg.m_Type = LRADDRESS_T;
		MemCopy (stack, frameArgs, arg, stack.Pop (), false);
	}

	Jmp (qContextInstr, m_Asm.m_Memory[qTargetInstr]);
}

TucanScript::VM::VirtualMachine::VirtualMachine (
	UInt64 stackSize, 
	UInt64 fixedMemSize, 
	UInt64 callDepth, 
	Asm&& asm_, 
	UnsafeDeallocator* staticDeallocator) :
	m_Stack (stackSize),
	m_Asm (std::move (asm_)),
	m_GlobalDeallocator (staticDeallocator),
	m_JmpMemory {
		.m_Sequence = new Call[NextWord (callDepth)],
		.m_Capacity = NextWord (callDepth),
		.m_Depth    = Zero
	},
	m_FixedMemory {
		.m_Memory = new Val[fixedMemSize],
		.m_Size   = fixedMemSize
	} {
	m_JmpMemory.ZeroOutMemory ();
}

TucanScript::VM::VirtualMachine::~VirtualMachine () {
	Free ();
}

Undef TucanScript::VM::VirtualMachine::Run (SInt64 entryPoint) {
	for (SInt64 qInstr = entryPoint; static_cast<QWORD>(qInstr) < m_Asm.m_Size && !m_Free; ++qInstr) {
		if (HandleInstr (qInstr, m_Stack, m_JmpMemory) == _Exit) {
			break;
		}
		for (QWORD qTask = Zero; qTask < m_TaskPool.GetCapacity (); qTask++) {
			lpTask task = m_TaskPool.GetTask (qTask);
			if (task->m_Running) {
				if (HandleInstr (task->m_qInstr, *task->m_Stack, task->m_Frame) == _Exit) {
					task->m_Running = false;
					delete task->m_Stack;
					task->m_Stack = nullptr;
					task->m_Frame.Free ();
				}
				else {
					task->m_qInstr++;
				}
			}
		}
	}
	Free ();
}

Undef TucanScript::VM::VirtualMachine::Free () {
	if (m_Free)
		return;

	delete[] m_Asm.m_Memory;
	delete[] m_FixedMemory.m_Memory;
	delete m_GlobalDeallocator;
	m_TaskPool.Free ();
	m_JmpMemory.Free ();
	m_Allocator.FreeAll ();
	m_Free = true;
}

Undef TucanScript::VM::TaskPool::Resize (Size newCapacity) {
	lpTask* newArray = new lpTask[newCapacity];
	for (QWORD qTask = Zero; qTask < Min(m_Capacity, newCapacity); ++qTask) {
		newArray[qTask] = m_Tasks[qTask];
	}
	delete[] m_Tasks;
	m_Tasks = newArray;
	m_Capacity = newCapacity;
}

TucanScript::VM::lpTask TucanScript::VM::TaskPool::Run (QWORD qInstr) {
	const Size frameBufferSize = NextWord (m_TaskMemoryProps.m_CallDepth);
	lpTask task;
	for (QWORD qTask = Zero; qTask < m_Capacity; qTask++) {
		task = m_Tasks[qTask];;
		if (!task->m_Running) {
			task->m_Running = true;
			task->m_qInstr = qInstr;
			
			delete task->m_Stack;
			task->m_Stack = new VMStack (m_TaskMemoryProps.m_StackSize);

			task->m_Frame.Free ();
			task->m_Frame.m_Depth    = Zero;
			task->m_Frame.m_Capacity = frameBufferSize,
			task->m_Frame.m_Sequence = new Call[frameBufferSize];
			task->m_Frame.ZeroOutMemory ();
			return task;
		}
	}
	task = new Task ();
	task->m_qInstr = qInstr;
	task->m_Running = true;
	task->m_Stack = new VMStack (m_TaskMemoryProps.m_StackSize);
	task->m_Frame = JmpMemory {
		.m_Sequence = new Call[frameBufferSize],
		.m_Capacity = frameBufferSize,
		.m_Depth    = Zero
	};
	task->m_Frame.ZeroOutMemory ();
	Resize (NextWord (m_Capacity));
	m_Tasks[PrevWord (m_Capacity)] = task;
	return task;
}

Undef TucanScript::VM::TaskPool::Free () {
	for (QWORD qTask = Zero; qTask < m_Capacity; ++qTask) {
		lpTask task = m_Tasks[qTask];
		delete task->m_Stack;
		task->m_Frame.Free ();
		delete task;
	}
	delete[] m_Tasks;
	m_Tasks = nullptr;
	m_Capacity = Zero;
}