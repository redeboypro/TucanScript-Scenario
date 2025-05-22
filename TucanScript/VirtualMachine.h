#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "Utility.h"

#define LogInstErr(INST, MSG)  std::cerr << INST ": " MSG << std::endl

namespace TucanScript::VM {
	union  Word;
	struct Val;
	struct Managed;
	class  VirtualMachine;

	typedef MemoryView<Val> ValMem;

	using ExCall_t = Undef (*)(VirtualMachine*, ValMem* const);

	enum OpCode : UInt8 {
		HALT,

		PUSH,
		POP,
		JMP,
		JMPC,		//Jump if false (Conditional jump)
		JMPR,       //Jump with recording
		RETURN,

		MEMSIZE,
		MEMLOAD,
		MEMSTORE,
		STRALLOC,
		SEQUENCEALLOC,
		MEMAPPEND,
		MEMALLOC,
		CMEMALLOC,
		MEMDEALLOC,
		MEMCPY,

		TOC,
		TOUC,
		TOU16,
		TOU32,
		TOU64,
		TOI16,
		TOI32,
		TOI64,
		TOF32,
		TOF64,

		ADD,
		SUB,
		MUL,
		DIV,

		CMPE,
		CMPNE,
		CMPG,
		CMPL,
		CMPGE,
		CMPLE,

		AND,
		OR,

		PRINT,
		SCAN,

		PTR2DWORD,
		PTR2QWORD,
		DWORD2PTR,
		QWORD2PTR,

		WRAP,

		CONCAT,
		STRCAT,
		STRCPY,

		LOADLIB,
		LOADSYM,
		DOEXCALL,
		GETMOD
	};

	enum ValType : UInt8 {
		CHAR_T,
		BYTE_T,
		UINT16_T,
		UINT32_T,
		UINT64_T,
		INT16_T,
		INT32_T,
		INT64_T,
		FLOAT32_T,
		FLOAT64_T,
		RADDRESS_T,
		LRADDRESS_T,
		MANAGED_T,
		NATIVEPTR_T
	};

	union Word {
		SInt8    m_C;
		UInt8    m_UC;
		UInt16   m_U16;
		UInt32   m_U32;
		UInt64   m_U64;
		SInt16   m_I16;
		SInt32   m_I32;
		SInt64   m_I64;
		Dec32    m_F32;
		Dec64    m_F64;
		Managed* m_ManagedPtr;
		Undef*   m_NativePtr;
	};

	static_assert (SizeEquals<Word, UInt64> (),
				   "Size of " nameof (TSData) " is not correct!");

#pragma pack(push, 1)
	struct Val final {
		mutable ValType m_Type;
		mutable Word    m_Data;
	};
#pragma pack(pop)

	namespace ValUtility {
		inline static Val _DWORD (SInt32 value, ValType type = INT32_T) {
			return Val {
				.m_Type = type,
				.m_Data = Word { .m_I32 = value }
			};
		}

		inline static Val _QWORD (UInt64 value, ValType type = UINT64_T) {
			return Val {
				.m_Type = type,
				.m_Data = Word { .m_U64 = value }
			};
		}

		static const Dictionary<ValType, Size> SizeMap =
		{
			{CHAR_T,      sizeof (SInt8)},
			{BYTE_T,      sizeof (UInt8)},
			{INT16_T,     sizeof (SInt16)},
			{INT32_T,     sizeof (SInt32)},
			{INT64_T,     sizeof (SInt64)},
			{UINT16_T,    sizeof (UInt16)},
			{UINT32_T,    sizeof (UInt32)},
			{UINT64_T,    sizeof (UInt64)},
			{FLOAT32_T,   sizeof (Dec32)},
			{FLOAT64_T,   sizeof (Dec64)},
			{MANAGED_T,   sizeof (Managed*)},
			{NATIVEPTR_T, sizeof (Undef*)},
			{RADDRESS_T,  sizeof (SInt32)},
			{LRADDRESS_T, sizeof (SInt32)},
		};
	};

	struct Managed final {
		Val*     m_Memory;
		UInt64   m_Size;
		SInt32   m_RefCount;
		Managed* m_Next;
		Managed* m_Previous;
	};

	struct Instruction final {
		OpCode m_Op;
		Val    m_Val;
	};

	using ReadOnlyData = MemoryView<Sym*>;
	using Asm          = MemoryView<Instruction>;

	inline Undef DeleteROData (const ReadOnlyData& roData) {
		for (Size iLiteral = Zero; iLiteral < roData.m_Size; iLiteral++) {
			delete[] roData.m_Memory[iLiteral];
		}
		delete[] roData.m_Memory;
	}

	struct Call final {
		ValMem m_Memory;
		QWORD  m_Address;
	};

	struct JmpMemory final {
		Call* m_Sequence;
		Size  m_Depth;
	};

	enum UnsafeMemoryType : UInt8 {
		STATIC_MEMORY,
		NATIVE_LIBRARY
	};

	struct UnsafeMemory final {
		UnsafeMemoryType m_Type;
		Undef*           m_Handle;
	};

	class UnsafeDeallocator final {
		UnsafeMemory* m_Handles;
		Size          m_NumHandles;

	public:
		UnsafeDeallocator ();
		~UnsafeDeallocator ();

		UInt64 GetSizeInBytes () const;
		Undef* operator[](UInt64 index);

		Undef PutHandle (const UnsafeMemory& handle);
		Undef PutReadOnlyData (const ReadOnlyData& roData);
		Undef Free ();
	};

	class VMStack final {
		Val* m_Data;
		SInt32 m_End;
	public:
		VMStack (Size size);
		~VMStack ();

		const Size m_Size;

		template<typename CTYPE, ValType TYPE>
		inline Undef Push (CTYPE value, CTYPE Word::* field) {
			Val result {
				.m_Type = TYPE
			};
			result.m_Data.*field = value;
			Push (result);
		}

		Undef Push (Val value);
		Undef Push (Boolean value);
		Undef Push (SInt8 value);
		Undef Push (UInt8 value);
		Undef Push (UInt16 value);
		Undef Push (UInt32 value);
		Undef Push (UInt64 value);
		Undef Push (SInt16 value);
		Undef Push (SInt32 value);
		Undef Push (SInt64 value);
		Undef Push (Dec32 value);
		Undef Push (Dec64 value);
		Val Pop ();
	};

	class VMAllocator final {
		Managed* m_Begin;
		Managed* m_End;
		UInt64   m_NumBlocks { Zero };

	public:
		VMAllocator ();

		const Managed& Begin ();
		const Managed& End ();

		Managed* Alloc (UInt64 size);
		Undef Free (Managed* ptr, Boolean removeReferences = true);

		inline Undef RemoveRef (Managed* ptr) {
			ptr->m_RefCount--;
			HandleReferences (ptr);
		}

		inline Undef HandleReferences (Managed* ptr) {
			if (ptr->m_RefCount <= Zero) {
				Free (ptr);
			}
		}

		Undef FreeAll ();
	};

	struct Task final {
		QWORD     m_qInstr;
		Boolean   m_Running;
		VMStack   m_Stack;
		JmpMemory m_Frame;
	};

	struct TaskPool final {

	};

	class VirtualMachine final {
		VMStack            m_Stack;
		VMAllocator        m_Allocator;
		Asm                m_Asm;
		UnsafeDeallocator* m_GlobalDeallocator;
		ValMem             m_FixedMemory;
		JmpMemory          m_JmpMemory;
		Boolean            m_Free {};

		VM::Val Unpack(JmpMemory& frame, const Val& value) const;
		VM::Val PopUnpack(VMStack& stack, JmpMemory& frame);
		Undef Jmp (Size& iInst, Instruction& instruction);

		inline Call& GetLastCall (JmpMemory& jmpMemory) const {
			return jmpMemory.m_Sequence[jmpMemory.m_Depth];
		}

		VM::Val * GetMemoryAtAddress(JmpMemory& frame, const Val& src, Val* defaultValue) const;

		inline UInt64 GetMemorySize (const Val& value) const {
			return value.m_Data.m_ManagedPtr->m_Size;
		}

		Val* GetMemoryAtRAddress (SInt32 address) const {
			return &m_FixedMemory.m_Memory[address];
		}

		Val * GetMemoryAtLRAddress (JmpMemory& frame, SInt32 address) const {
			return &GetLastCall (frame).m_Memory.m_Memory[address];
		}

		Undef FreeManagedMemory (Val* memory);

		template<typename TYPE>
		Boolean TryCast (Val& value, TYPE Word::* sourceField) {
			switch (value.m_Type) {
				case CHAR_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_C);
				break;
				case BYTE_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_UC);
				break;
				case UINT16_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_U16);
				break;
				case UINT32_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_U32);
				break;
				case UINT64_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_U64);
				break;
				case INT16_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_I16);
				break;
				case INT32_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_I32);
				break;
				case INT64_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_I64);
				break;
				case FLOAT32_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_F32);
				break;
				case FLOAT64_T:
				value.m_Data.*sourceField = static_cast<TYPE>(value.m_Data.m_F64);
				break;
				case NATIVEPTR_T:
				value.m_Data.*sourceField = *reinterpret_cast<TYPE*>(value.m_Data.m_NativePtr);
				break;
				default: {
					LogInstErr ("CAST", "Invalid type to cast!");
					Free ();
					return false;
				}
			}
			return true;
		}

		template<typename TYPE>
		inline Undef Cast (Val& data, ValType type, TYPE Word::* sourceField) {
			if (TryCast<TYPE> (data, sourceField)) {
				data.m_Type = type;
				m_Stack.Push (data);
				return;
			}
			LogInstErr ("CAST", "Unable to cast!");
			Free ();
		}

		template<typename TYPE>
		Undef Cast (ValType type, TYPE Word::* sourceField) {
			auto poppedValue = m_Stack.Pop ();
			if (poppedValue.m_Type Is RADDRESS_T) {
				Cast<TYPE> (*GetMemoryAtRAddress (poppedValue.m_Data.m_I32), type, sourceField);
			}
			else if (poppedValue.m_Type Is LRADDRESS_T) {
				Cast<TYPE> (*GetMemoryAtLRAddress(_placeholder_, poppedValue.m_Data.m_I32), type, sourceField);
			}
			else {
				Cast<TYPE> (poppedValue, type, sourceField);
			}
		}

		Undef Print (const Val& value, Boolean flush = true, Boolean handleReferences = true) {
			switch (value.m_Type) {
				case CHAR_T:
				std::cout << value.m_Data.m_C;
				break;
				case BYTE_T:
				std::cout << value.m_Data.m_UC;
				break;
				case UINT16_T:
				std::cout << value.m_Data.m_U16;
				break;
				case UINT32_T:
				std::cout << value.m_Data.m_U32;
				break;
				case UINT64_T:
				std::cout << value.m_Data.m_U64;
				break;
				case INT16_T:
				std::cout << value.m_Data.m_I16;
				break;
				case INT32_T:
				std::cout << value.m_Data.m_I32;
				break;
				case INT64_T:
				std::cout << value.m_Data.m_I64;
				break;
				case FLOAT32_T:
				std::cout << value.m_Data.m_F32;
				break;
				case FLOAT64_T:
				std::cout << value.m_Data.m_F64;
				break;
				case MANAGED_T: {
					auto* managedPtr = value.m_Data.m_ManagedPtr;
					for (SInt32 iElement = Zero; iElement < managedPtr->m_Size; iElement++) {
						Print (managedPtr->m_Memory[iElement], false, false);
					}

					if (handleReferences) {
						m_Allocator.HandleReferences (managedPtr);
					}
				}
				break;
				case NATIVEPTR_T:
				std::cout << reinterpret_cast<Sym*>(value.m_Data.m_NativePtr);
				break;
			}
			if (flush) {
				std::flush (std::cout);
			}
		}

		Undef MemCopy(VMStack& stack, JmpMemory& frame, const Val& dest, const Val& src, Boolean pushBack = true);

		inline Boolean IsTrue (const Val& value) const {
			return (value.m_Type Is BYTE_T  && value.m_Data.m_UC) ||
				   (value.m_Type Is CHAR_T  && value.m_Data.m_C)  ||
				   (value.m_Type Is INT32_T && value.m_Data.m_I32);
		}

		Undef StrOp(VMStack& stack, const Val& a, const Val& b, Sym* (*op)(Sym*, const Sym*));
		Sym* GetCStr (const Managed* managedMemory);
		Undef AllocStr(VMStack& stack, Sym* buffer, Size size);

		Undef HandleInstr(QWORD& qInstr, VMStack& stack, JmpMemory& frame);

	public:
		VirtualMachine (
			UInt64 stackSize,
			UInt64 fixedMemSize,
			SInt32 callDepth,
			Asm&& asm_,
			UnsafeDeallocator* staticDeallocator);

		~VirtualMachine ();

		Undef Run (SInt32 entryPoint = Zero);
		Undef Free ();

		VMStack* GetStack () { return &m_Stack; };
	};
}
#endif