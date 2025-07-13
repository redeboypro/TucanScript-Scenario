#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "Utility.h"

#define LogInstErr(INST, MSG)  std::cerr << INST ": " MSG << std::endl

#define _Success 0x1ll
#define _Fail    -(_Success)
#define _Exit    _Fail

namespace TucanScript::VM {
	union  Word;
	struct Val;
	struct Managed;
	struct JmpMemory;
	class  VirtualStack;
	class  VirtualMachine;

	typedef MemoryView<Val> ValMem;

	using ExternCall_t = Undef (*)(VirtualMachine*, VirtualStack*, JmpMemory*, ValMem* const);

	#define ExC_Args VM::VirtualMachine* vm, \
					 VM::VirtualStack* stack,\
					 VM::JmpMemory* frame,   \
					 const VM::ValMem* args

	enum OpCode : UInt8 {
		HALT,

		PUSH,
		POP,
		JMP,
		JMPC,		//Jump if false (Conditional jump)
		JMPR,       //Jump with recording
		CALLASYNC,  //Start coroutine
		RETURN,

		MEMSIZE,
		MEMLOAD,
		MEMSTORE,
		STRALLOC,
		CSTRALLOC,
		SEQUENCEALLOC,
		MEMAPPEND,
		MEMALLOC,
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

		SIN,
		COS,
		ATAN2,
		SQRT,
		ABSF,

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

		ADDR,
		PIN,

		LOADLIB,
		LOADSYM,
		CALLADDR,
		GETMOD,

		YIELD
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
		inline static Val _DWORD_signed_raw (Undef* value, Boolean fromUnsigned) {
			UInt32 raw;
			std::memcpy (&raw, value, sizeof (raw));
			SInt32 signedWord = fromUnsigned ? static_cast<SInt32>(raw) : std::bit_cast<SInt32>(raw);

			Word wordDef {};
			std::memcpy (&wordDef, &signedWord, sizeof (signedWord));

			return Val {
				.m_Type = INT32_T,
				.m_Data = wordDef
			};
		}

		inline static Val _QWORD_signed_raw (Undef* value, Boolean fromUnsigned) {
			UInt64 raw;
			std::memcpy (&raw, value, sizeof (raw));
			SInt64 signedWord = fromUnsigned ? static_cast<SInt64>(raw) : std::bit_cast<SInt64>(raw);

			Word wordDef {};
			std::memcpy (&wordDef, &signedWord, sizeof (signedWord));

			return Val {
				.m_Type = INT64_T,
				.m_Data = wordDef
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

	union MemoryVariant {
		Val*   m_hSpecBuf;
		Undef* m_hRawBuf;
	};

	enum ManagedType { SPECIFIC_T, RAW_T };

	struct Managed final {
		ManagedType   m_MemoryType;
		MemoryVariant m_Memory;
		UInt64        m_Size;
		SInt32        m_RefCount;
		Managed*      m_hNext;
		Managed*      m_hPrevious;
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
		SInt64 m_Address;
	};

	struct JmpMemory final {
		Call* m_Sequence;
		Size  m_Capacity;
		Size  m_Depth;

		inline Undef ZeroOutMemory () {
			if (!m_Sequence)
				return;

			for (QWord qCall = Zero; qCall < m_Capacity; qCall++) {
				m_Sequence[qCall].m_Memory.m_Memory = nullptr;
			}
		}
		
		inline Undef Free () {
			if (!m_Sequence)
				return;

			for (QWord qCall = Zero; qCall < m_Capacity; qCall++) {
				Call& call = m_Sequence[qCall];
				delete[] call.m_Memory.m_Memory;
				call.m_Memory.m_Memory = nullptr;
			}

			delete[] m_Sequence;
			m_Sequence = nullptr;
			m_Capacity = Zero;
		}
	};

	enum UnsafeMemoryType : UInt8 { READONLY_MEMORY, MODULE_HANDLE };

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

	class VirtualStack final {
		Val* m_Data;
		SInt32 m_End;
	public:
		VirtualStack (Size size);
		~VirtualStack ();

		const Size m_Size;

		template<typename CTYPE, ValType TYPE>
		inline Undef Push (CTYPE value, CTYPE Word::* field) {
			Val result {
				.m_Type = TYPE
			};
			result.m_Data.*field = value;
			Push(result);
		}

		Undef Push(const Val& value);
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

	class VirtualAllocator final {
		Managed* m_hBegin;
		Managed* m_hEnd;
		UInt64   m_nBlocks { Zero };

		inline Managed* Pin (Managed* allocated) {
			if (m_hEnd) {
				m_hEnd->m_hNext = allocated;
			}

			m_hEnd = allocated;

			if (!m_hBegin) {
				m_hBegin = allocated;
			}

			m_nBlocks++;

			return allocated;
		}

	public:
		VirtualAllocator ();

		const Managed& Begin ();
		const Managed& End ();

		Managed* Alloc (UInt64 size);
		Managed* Alloc (Undef* rawMemory, Size size);

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
		SInt64        m_qInstr;
		Boolean       m_Running;
		VirtualStack* m_hStack;
		JmpMemory     m_Frame;
	};

	using HTask = Task*;

	class TaskScheduler final {
		HTask*  m_Tasks { nullptr };
		Size    m_Capacity { Zero };

		typedef struct {
			Size m_StackSize;
			Size m_CallDepth;
		} TaskMemoryProps;

		Undef Resize (Size newCapacity);

	public:
		TaskScheduler () = default;

		~TaskScheduler () {
			Free ();
		}

		HTask Run (QWord qInstr);

		constexpr Size GetCapacity () const {
			return m_Capacity;
		}

		HTask GetTask (QWord qTask) const {
			return m_Tasks[qTask];
		}

		Undef Free ();

		TaskMemoryProps m_TaskMemoryProps { 128ULL, 100ULL };
	};

	struct MemCpyFrameArgs final {
		JmpMemory* m_hSrcFrame { nullptr };
		JmpMemory* m_hDestFrame { nullptr };

		MemCpyFrameArgs () = default;
		MemCpyFrameArgs (JmpMemory* ptr) : m_hSrcFrame (ptr), m_hDestFrame (ptr) {}
	};

	class VirtualMachine final {
		VirtualStack            m_Stack;
		VirtualAllocator        m_Allocator;
		Asm						m_Asm;
		UnsafeDeallocator*		m_hGlobalDeallocator;
		ValMem					m_FixedMemory;
		JmpMemory				m_JmpMemory;
		TaskScheduler           m_TaskPool;
		Boolean					m_Free {};
		SInt64					m_IPtr;

		VM::Val Unpack (JmpMemory& frame, const Val& value) const;
		VM::Val PopUnpack (VirtualStack& stack, JmpMemory& frame);
		Undef Jmp (SInt64 & iInst, Instruction& instruction);

		inline Call& GetLastCall (JmpMemory& jmpMemory) const {
			return jmpMemory.m_Sequence[jmpMemory.m_Depth];
		}

		VM::Val* GetMemoryAtAddress (JmpMemory& frame, const Val& src, Val* defaultValue) const;

		inline UInt64 GetMemorySize (const Val& value) const {
			return value.m_Data.m_ManagedPtr->m_Size;
		}

		Val* GetMemoryAtRAddress (SInt32 address) const {
			return &m_FixedMemory.m_Memory[address];
		}

		Val* GetMemoryAtLRAddress (JmpMemory& frame, SInt32 address) const {
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
				m_Stack.Push(data);
				return;
			}
			LogInstErr ("CAST", "Unable to cast!");
			Free ();
		}

		template<typename TYPE>
		Undef Cast (JmpMemory& frame, ValType type, TYPE Word::* sourceField) {
			auto poppedValue = m_Stack.Pop ();
			if (poppedValue.m_Type Is RADDRESS_T) {
				Cast<TYPE> (*GetMemoryAtRAddress (poppedValue.m_Data.m_I32), type, sourceField);
			}
			else if (poppedValue.m_Type Is LRADDRESS_T) {
				Cast<TYPE> (*GetMemoryAtLRAddress(frame, poppedValue.m_Data.m_I32), type, sourceField);
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

					if (managedPtr->m_MemoryType == SPECIFIC_T) {
						for (SInt32 iElement = Zero; iElement < managedPtr->m_Size; iElement++) {
							Print (managedPtr->m_Memory.m_hSpecBuf[iElement], false, false);
						}
					}
					else {
						std::cout << (Sym*) managedPtr->m_Memory.m_hRawBuf;
						std::flush (std::cout);
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

		Undef MemCopy(VirtualStack& stack, MemCpyFrameArgs frameArgs, Val & dest, const Val& src, Boolean pushBack = true);

		inline Boolean IsTrue (const Val& value) const {
			return (value.m_Type Is BYTE_T  && value.m_Data.m_UC)   ||
				   (value.m_Type Is CHAR_T  && value.m_Data.m_C)    ||
				   (value.m_Type Is INT32_T && value.m_Data.m_I32)  ||
				   (value.m_Type Is UINT32_T && value.m_Data.m_I32) ||
				   (value.m_Type Is NATIVEPTR_T && value.m_Data.m_NativePtr);
		}

		Sym* GetCStr (const Managed* managedMemory);
		Undef AllocStr (VirtualStack& stack, Sym* buffer, Size size);

		TucanScript::SInt32 HandleInstr (SInt64& qInstr, VirtualStack& stack, JmpMemory& frame);

		Undef DoRecordJump (SInt64& qContextInstr, Instruction& jmpInstr,
			VirtualStack& stack,
			const MemCpyFrameArgs& frameArgs);

		Val GetRawElement (Managed* managedMemory, UInt64 index) {
			auto rawMemory =
			#if CHAR_MIN < 0
				(SInt8*) managedMemory;
			#else
				(UInt8*) managedMemory;
			#endif
			return Val {
				.m_Type = NATIVEPTR_T,
				.m_Data = Word { 
					.m_NativePtr = &(rawMemory)[index]
				}
			};
		}

		inline Undef LinearAlgProc (
			VirtualStack& stack,
			JmpMemory& frame,
			Dec32 (*f32)(Dec32), Dec64 (*f64)(Dec64)) {
			auto angle = PopUnpack (stack, frame);
			if (angle.m_Type Is ValType::FLOAT32_T) {
				stack.Push (f32 (angle.m_Data.m_F32));
			}
			else if (angle.m_Type Is ValType::FLOAT64_T) {
				stack.Push (f64 (angle.m_Data.m_F64));
			}
		}

	public:
		VirtualMachine (
			UInt64 stackSize,
			UInt64 fixedMemSize,
			UInt64 callDepth,
			Asm&& asm_,
			UnsafeDeallocator* hStaticDeallocator);

		~VirtualMachine ();

		Undef Run ();
		Undef Free ();

		inline Undef ForceCall (SInt64 destInstrPtr) {
			Instruction jmpInstr {
				.m_Op  = JMPR,
				.m_Val = Val {
					.m_Type = ValType::INT64_T,
					.m_Data = Word {
						.m_I64 = destInstrPtr
					}
				}
			};
			DoRecordJump (m_IPtr, jmpInstr, m_Stack, &m_JmpMemory);
			Skip ();
		}

		Undef WaitForYield ();
		Undef ResumeTask (HTask hTask);
		inline Undef CloseTask (HTask hTask) {
			hTask->m_Running = false;
			delete hTask->m_hStack;
			hTask->m_hStack = nullptr;
			hTask->m_Frame.Free ();
		}

		inline Boolean Next () {
			if (HandleInstr (m_IPtr, m_Stack, m_JmpMemory) == _Exit) {
				return false;
			}
			else {
				m_IPtr++;
				return true;
			}
		}

		inline Boolean Skip () {
			if (!IPtrIsOutOfProgram ()) {
				m_IPtr++;
				return true;
			}
			else {
				return false;
			}
		}

		inline Boolean IPtrIsOutOfProgram () const {
			return m_IPtr < Zero || m_IPtr >= m_Asm.m_Size || m_Free;
		}

		inline Undef ForceExitProgram () { 
			m_IPtr = _Exit; 
		}

		inline Undef WriteChunk (Size qChunkPtr, const Val& chunkVal) {
			m_FixedMemory.m_Memory[qChunkPtr] = chunkVal;
		}

		VirtualStack* GetStack () { return &m_Stack; };
		VirtualAllocator* GetAllocator () { return &m_Allocator; };
		TaskScheduler* GetScheduler () { return &m_TaskPool; };
	};
}
#endif