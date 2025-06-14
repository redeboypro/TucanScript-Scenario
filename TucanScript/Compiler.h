#ifndef COMPILER_H
#define COMPILER_H

#include "Lexer.h"
#include "VirtualMachine.h"

#define RangeInclude(VAL, FIRST, LAST) ((VAL) >= (FIRST) && (VAL) <= (LAST))

#define LowestPrecedence       0x0
#define OrPrecedence           0x1
#define AndPrecedence          0x2
#define CmpPrecedence          0x3
#define ArithmeticPrecedenceF1 0x4
#define ArithmeticPrecedenceF2 0x5

#define IsUndefined(TYPE) ((TYPE) Is Lexer::TokenType::UNDEFINED)
#define IsLParen(TYPE)    ((TYPE) Is Lexer::TokenType::LPAREN)
#define IsRParen(TYPE)    ((TYPE) Is Lexer::TokenType::RPAREN)
#define IsComma(TYPE)     ((TYPE) Is Lexer::TokenType::COMMA)
#define IsLBrace(TYPE)    ((TYPE) Is Lexer::TokenType::LBRACE)
#define IsRBrace(TYPE)    ((TYPE) Is Lexer::TokenType::RBRACE)

#define OpenCounter      1
#define InvalidSignature InvalidID

namespace TucanScript {
	using VarSet = Vector<String>;
	struct FuncInfo final {
		VarSet         m_DefinedVars;
		SInt32         m_NumArgs {};
		SInt64         m_Address {};
		Vector<QWord>  m_Calls {};
	};

	struct WhileInfo final {
		QWord         m_Entry {};
		Vector<QWord> m_BreakPoints;
	};

	struct InvokableOp final {
		VM::OpCode m_Operation {};
		Boolean    m_PushNumArgs {};

		InvokableOp () = default;
		InvokableOp (VM::OpCode op) : m_Operation (op) {}
		InvokableOp (VM::OpCode op, Boolean pushNumArgs) : m_Operation (op), m_PushNumArgs (pushNumArgs) {}
	};

	class Compiler final {
		Vector<String>               m_StringLiterals;
		Dictionary<String, FuncInfo> m_DefinedFuncs;
		VarSet                       m_DefinedVars;
		Vector<VM::Instruction>      m_Instructions;
		UInt64                       m_NumAutoPtrs {};
		WhileInfo                    m_EmptyWhileStmt;

		inline SInt32 Precedence (Lexer::TokenType type) {
			switch (type) {
				case Lexer::TokenType::CMPE:
				case Lexer::TokenType::CMPG:
				case Lexer::TokenType::CMPL:
				case Lexer::TokenType::CMPGE:
				case Lexer::TokenType::CMPLE:
				case Lexer::TokenType::NOT:
				return CmpPrecedence;

				case Lexer::TokenType::PLUS:
				case Lexer::TokenType::MINUS:
				return ArithmeticPrecedenceF1;

				case Lexer::TokenType::MUL:
				case Lexer::TokenType::DIV:
				return ArithmeticPrecedenceF2;

				case Lexer::TokenType::AND:
				return AndPrecedence;

				case Lexer::TokenType::OR:
				return OrPrecedence;

				default:
				return LowestPrecedence;
			}
		}

		template <typename TYPE>
		inline Undef Push (TYPE value, VM::ValType type, TYPE VM::Word::* sourceField) {
			VM::Word data {};
			data.*sourceField = value;
			m_Instructions.push_back (VM::Instruction {
				.m_Op  = VM::PUSH,
				.m_Val = VM::Val{
					.m_Type = type,
					.m_Data = data
				}
			});
		}

		inline Undef Push (SInt8 value) {
			Push<SInt8> (value, VM::CHAR_T, &VM::Word::m_C);
		}

		inline Undef Push (UInt8 value) {
			Push<UInt8> (value, VM::BYTE_T, &VM::Word::m_UC);
		}

		inline Undef Push (SInt16 value) {
			Push<SInt16> (value, VM::INT16_T, &VM::Word::m_I16);
		}

		inline Undef Push (SInt32 value) {
			Push<SInt32> (value, VM::INT32_T, &VM::Word::m_I32);
		}

		inline Undef Push (SInt32 value, VM::ValType type) {
			Push<SInt32> (value, type, &VM::Word::m_I32);
		}

		inline Undef Push (SInt64 value) {
			Push<SInt64> (value, VM::INT64_T, &VM::Word::m_I64);
		}

		inline Undef Push (UInt16 value) {
			Push<UInt16> (value, VM::UINT16_T, &VM::Word::m_U16);
		}

		inline Undef Push (UInt32 value) {
			Push<UInt32> (value, VM::UINT32_T, &VM::Word::m_U32);
		}

		inline Undef Push (UInt64 value) {
			Push<UInt64> (value, VM::UINT64_T, &VM::Word::m_U64);
		}

		inline Undef Push (Dec32 value) {
			Push<Dec32> (value, VM::FLOAT32_T, &VM::Word::m_F32);
		}

		inline Undef Push (Dec64 value) {
			Push<Dec64> (value, VM::FLOAT64_T, &VM::Word::m_F64);
		}

		inline VM::Instruction& Op (Lexer::TokenType type) {
			return m_Instructions.emplace_back (VM::Instruction {
				.m_Op = OpMap.at (type)
			});
		}

		inline VM::Instruction& Op (VM::OpCode op) {
			return m_Instructions.emplace_back (VM::Instruction {
				.m_Op = op
			});
		}

		inline Undef Alloc (QWord size) {
			m_Instructions.push_back (VM::Instruction {
				.m_Op  = VM::SEQUENCEALLOC,
				.m_Val = VM::Val {
					.m_Type = VM::UINT64_T,
					.m_Data = VM::Word {
						.m_U64 = size
					}
				}
			});
		}

		inline Undef StringAlloc (const String& str, Boolean native) {
			auto address = static_cast<QWord>(Found (m_StringLiterals, str));
			if (!IsValidID (address)) {
				address = m_StringLiterals.size ();
				m_StringLiterals.push_back (str);
			}

			m_Instructions.push_back (VM::Instruction {
				.m_Op  = native ? VM::CSTRALLOC : VM::STRALLOC,
				.m_Val = VM::Val {
					.m_Type = VM::UINT64_T,
					.m_Data = VM::Word {
						.m_U64 = address
					}
				}
			});
		}

		inline Undef Call (FuncInfo& funInfo, Boolean async = false) {
			funInfo.m_Calls.push_back (m_Instructions.size ());
			m_Instructions.push_back (VM::Instruction {
				.m_Op  = async ? VM::CALLASYNC : VM::JMPR,
				.m_Val = VM::Val { .m_Type = VM::ValType::INT64_T }
			});
		}

		inline VM::Val GetInstrEnd (QWord offset = Zero) {
			QWord instrWord = m_Instructions.size () + offset;
			return VM::ValUtility::_QWORD_signed_raw (&instrWord, true);
		}

		inline Boolean IsLeftBracket (const Lexer::Token& token) {
			return token.m_Type Is Lexer::TokenType::LPAREN ||
				   token.m_Type Is Lexer::TokenType::LBRACE ||
				   token.m_Type Is Lexer::TokenType::LSQRBRACKET;
		}

		inline Boolean IsRightBracket (const Lexer::Token& token) {
			return token.m_Type Is Lexer::TokenType::RPAREN ||
				   token.m_Type	Is Lexer::TokenType::RBRACE ||
				   token.m_Type Is Lexer::TokenType::RSQRBRACKET;
		}

		Undef GenerateInstructionList (
			Lexer::TokenList rawTokens, 
			VarSet& varSet, 
			Boolean externalContext, 
			WhileInfo& lastWhileStmt);

		SInt32 ProcArgs (
			Lexer::TokenList& inTokens, 
			SInt32& iToken, 
			VarSet& varSet, 
			Boolean externalContext, 
			Lexer::TokenType opener = Lexer::TokenType::LPAREN);

		Undef ReadTo (
			Lexer::TokenType itBreaker, 
			Lexer::TokenList& inTokens, 
			SInt32& iToken, 
			Lexer::TokenList& outTokens);

		Undef ProcStatement (
			Lexer::TokenList& inTokens, 
			SInt32& iToken, 
			Lexer::TokenList& outTokens, 
			Boolean stopOnBreaker = true);

		Undef ProcExpression (
			Lexer::TokenList expressionTokens, 
			VarSet& varSet, 
			Boolean externalContext, 
			Boolean innerExpr = false);

		const Dictionary<Lexer::TokenType, VM::OpCode> OpMap {
			{ Lexer::TokenType::CPY,   VM::MEMCPY },
			{ Lexer::TokenType::MUL,   VM::MUL },
			{ Lexer::TokenType::DIV,   VM::DIV },
			{ Lexer::TokenType::PLUS,  VM::ADD },
			{ Lexer::TokenType::MINUS, VM::SUB },
			{ Lexer::TokenType::CMPG,  VM::CMPG },
			{ Lexer::TokenType::CMPL,  VM::CMPL },
			{ Lexer::TokenType::CMPE,  VM::CMPE },
			{ Lexer::TokenType::CMPGE, VM::CMPGE },
			{ Lexer::TokenType::CMPLE, VM::CMPLE },
			{ Lexer::TokenType::CMPNE, VM::CMPNE },
			{ Lexer::TokenType::AND,   VM::AND },
			{ Lexer::TokenType::OR,    VM::OR },
			{ Lexer::TokenType::WRITE, VM::MEMSTORE },
		};

		const Dictionary<String, InvokableOp> InvokableOpMap {
			{ "sbyte",           VM::TOC },
			{ "ubyte",           VM::TOUC },
			{ "short",           VM::TOI16 },
			{ "int",             VM::TOI32 },
			{ "long",            VM::TOI64 },
			{ "ushort",          VM::TOU16 },
			{ "uint",            VM::TOU32 },
			{ "DWORD",           VM::TOU32 },
			{ "ulong",           VM::TOU64 },
			{ "QWORD",           VM::TOU64 },
			{ "float",           VM::TOF32 },
			{ "double",          VM::TOF64 },

			{ "PtrAsDWORD",      VM::PTR2DWORD },
			{ "PtrAsQWORD",      VM::PTR2QWORD },
			{ "DWORDAsPtr",      VM::DWORD2PTR },
			{ "QWORDAsPtr",      VM::QWORD2PTR },

			{ "Log",             VM::PRINT },
			{ "Scan",            VM::SCAN },
			{ "PopStack",        VM::POP },
			{ "SetTaskProps",    VM::SETTASKPROPS },
			{ "Alloc",           VM::MEMALLOC },
			{ "Pin",             { VM::PIN, true } },
			{ "GetRawPtr",       VM::GETRAWMEM },
			{ "Append",          VM::MEMAPPEND },
			{ "Free",            VM::MEMDEALLOC },
			{ "Size",            VM::MEMSIZE },
			{ "Return",          { VM::RETURN, true } },
			{ "IntPtr",          VM::WRAP },
			{ "Join",            VM::CONCAT },
			{ "Exit",            VM::HALT },

			//C Functions
			{ "malloc",          VM::CMEMALLOC },
			{ "strcat",          VM::STRCAT },
			{ "strcpy",          VM::STRCPY },

			//Native wrapping
			{ "LoadLibrary",     VM::LOADLIB },
			{ "GetProcAddr",     VM::LOADSYM },
			{ "Yield",           VM::YIELD },
		};

		const Dictionary<VM::OpCode, String> OpDebugMap {
			{VM::HALT,          "Halt"},
			{VM::PUSH,          "Push"},
			{VM::POP,           "Pop"},
			{VM::JMP,           "Jmp"},
			{VM::JMPC,          "Jmp (Conditional)"},
			{VM::JMPR,          "Jmp (Call recording)"},
			{VM::CALLASYNC,     "CallAsync"},
			{VM::RETURN,        "Return"},
			{VM::MEMSIZE,       "MemorySize"},
			{VM::MEMLOAD,       "MemoryLoad"},
			{VM::MEMSTORE,      "MemoryStore"},
			{VM::MEMAPPEND,     "MemoryAppend"},
			{VM::STRALLOC,      "MemoryAlloc (String)"},
			{VM::CSTRALLOC,     "NativeMemoryAlloc (String)"},
			{VM::SEQUENCEALLOC, "MemoryAlloc (Sequence)"},
			{VM::CMEMALLOC,     "NativeMemoryAlloc"},
			{VM::MEMALLOC,      "MemoryAlloc"},
			{VM::MEMDEALLOC,    "MemoryDealloc"},
			{VM::MEMCPY,        "MemoryCopy"},
			{VM::TOC,           "ToChar"},
			{VM::TOUC,          "ToByte"},
			{VM::TOU16,         "ToUInt16"},
			{VM::TOU32,         "ToUInt32"},
			{VM::TOU64,         "ToUInt64"},
			{VM::TOI16,         "ToInt16"},
			{VM::TOI32,         "ToInt32"},
			{VM::TOI64,         "ToInt64"},
			{VM::TOF32,         "ToDec32"},
			{VM::TOF64,         "ToDec64"},
			{VM::ADD,           "Add"},
			{VM::SUB,           "Sub"},
			{VM::MUL,           "Mul"},
			{VM::DIV,           "Div"},
			{VM::CMPE,          "Compare (Equals)"},
			{VM::CMPNE,         "Compare (Not equals)"},
			{VM::CMPG,          "Compare (Greater)"},
			{VM::CMPL,          "Compare (Less)"},
			{VM::CMPGE,         "Compare (Greater or equals)"},
			{VM::CMPLE,         "Compare (Less or equals)"},
			{VM::AND,           "And"},
			{VM::OR,            "Or"},
			{VM::PRINT,         "SysOut"},
			{VM::SCAN,          "SysIn"},
			{VM::PTR2DWORD,     "PtrToDWORD"},
			{VM::PTR2QWORD,     "PtrToQWORD"},
			{VM::DWORD2PTR,     "DWORDToPtr"},
			{VM::QWORD2PTR,     "QWORDToPtr"},
			{VM::WRAP,          "Pack"},
			{VM::CONCAT,        "Concat"},
			{VM::STRCAT,        "NativeConcat"},
			{VM::STRCPY,        "NativeStringCopy"},
			{VM::HALT,          "Halt"},
			{VM::LOADLIB,       "LoadLibrary"},
			{VM::LOADSYM,       "LoadSym"},
			{VM::DOEXCALL,      "ExternalCall"},
			{VM::PIN,           "PinMemoryChunk"},
			{VM::GETRAWMEM,     "GetRawMemoryPtr"},
			{VM::SETTASKPROPS,  "SetTaskAllocProps"},
			{VM::YIELD,         "Yield"}
		};

	public:
		inline Undef GenerateInstructionList (Lexer::TokenList rawTokens) {
			GenerateInstructionList (rawTokens, m_DefinedVars, true, m_EmptyWhileStmt);
			for (auto& funPair : m_DefinedFuncs) {
				auto& funInfo = funPair.second;
				for (auto inst : funInfo.m_Calls) {
					m_Instructions[inst].m_Val.m_Data.m_I64 = funInfo.m_Address;
				}
			}
		}

		inline Undef DefineVar (const String& variableName) {
			m_DefinedVars.push_back (variableName);
		}

		template<typename... Args>
		inline Undef DefineFunc (const String& funName, const Args&... variables) {
			static_assert((std::is_same_v<Args, String> && ...), "All arguments must be String");

			FuncInfo fun {};
			fun.m_Address = InvalidID;
			(fun.m_DefinedVars.push_back (variables), ...);

			m_DefinedFuncs.emplace (funName, std::move (fun));
		}

		inline Undef LogFuncTable () {
			for (auto& [funName, funInfo] : m_DefinedFuncs) {
				Log (funName << ": " << funInfo.m_Address);
			}
		}
		
		inline SInt64 GetFuncAddr (const String& functionName) {
			auto it = m_DefinedFuncs.find (functionName);
			if (it != m_DefinedFuncs.end ()) {
				return it->second.m_Address;
			}
			return InvalidID;
		}

		VM::ReadOnlyData GetReadOnlyData ();
		VM::Asm GetAssemblyCode ();
		Undef LogInstr ();
	};
}

#endif