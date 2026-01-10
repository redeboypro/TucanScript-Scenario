#include "Compiler.h"

using namespace TucanScript;

#define LogOpenStream(DATA)  std::cout << (DATA)
#define Log2OpenStream(A, B) std::cout << "(" << (A) << "; " << (B) << ")"
Undef Compiler::LogInstr () const {
	for (auto& instr : m_Instructions) {
		auto& value = instr.m_Val;
		LogOpenStream (OpDebugMap.at (instr.m_Op) + ": ");
		switch (value.m_Type) {
			case VM::CHAR_T:
			Log2OpenStream ("Char", value.m_Data.m_C);
			break;
			case VM::BYTE_T:
			Log2OpenStream ("Byte", value.m_Data.m_UC);
			break;
			case VM::UINT16_T:
			Log2OpenStream ("UInt16", value.m_Data.m_U16);
			break;
			case VM::UINT32_T:
			Log2OpenStream ("UInt32", value.m_Data.m_U32);
			break;
			case VM::UINT64_T:
			Log2OpenStream ("UInt64", value.m_Data.m_U64);
			break;
			case VM::INT16_T:
			Log2OpenStream ("Int16", value.m_Data.m_I16);
			break;
			case VM::INT32_T:
			Log2OpenStream ("Int32", value.m_Data.m_I32);
			break;
			case VM::INT64_T:
			Log2OpenStream ("Int64", value.m_Data.m_I64);
			break;
			case VM::FLOAT32_T:
			Log2OpenStream ("Dec32", value.m_Data.m_F32);
			break;
			case VM::FLOAT64_T:
			Log2OpenStream ("Dec64", value.m_Data.m_F64);
			break;
			case VM::RADDRESS_T:
			Log2OpenStream ("RelativeAddress", value.m_Data.m_I32);
			break;
			case VM::LRADDRESS_T:
			Log2OpenStream ("LocalRelativeAddress", value.m_Data.m_I32);
			break;
			case VM::NATIVEPTR_T:
			Log2OpenStream ("NativePtr", value.m_Data.m_NativePtr);
			break;
		}
		LogOpenStream (LineSeparators::NextLine);
	}
	std::flush (std::cout);
}
#undef LogOpenStream
#undef Log2OpenStream

Undef Compiler::GenerateInstructionList (Lexer::TokenList rawTokens, VarSet& varSet, Boolean externalContext, WhileInfo& lastWhileStmt) {
	Lexer::TokenList argTokens;
	for (SInt32 iToken = 0; iToken < rawTokens.size (); iToken++) {
		switch (auto& curToken = rawTokens[iToken]; curToken.m_Type) {
			case Lexer::TokenType::UNDEFINED: {
				argTokens.clear ();
				ReadTo (Lexer::TokenType::SEMICOLON, rawTokens, iToken, argTokens);
				ProcExpression (argTokens, varSet, externalContext);
				break;
			}
			case Lexer::TokenType::CBUFFER: {
				argTokens.clear ();
				ReadTo (Lexer::TokenType::SEMICOLON, rawTokens, ++iToken, argTokens);

				SInt32 iSubToken = InvalidID;
				const SInt32 nArgs = ProcArgs (argTokens, iSubToken,
					varSet, externalContext, Lexer::TokenType::LBRACE);

				curToken = argTokens[++iSubToken];
				if (curToken.m_Type != Lexer::TokenType::UNDEFINED) break;

				auto iDefinedVar = static_cast<SInt32>(varSet.size());
				varSet.push_back(std::get<String>(curToken.m_Val));

				Push(nArgs);
				Push(iDefinedVar, externalContext ? VM::ValType::RADDRESS_T : VM::ValType::LRADDRESS_T);
				ProcArgs (argTokens, iSubToken, varSet, externalContext);
				Op(VM::CBUFFERALLOC);
				break;
			}
#define LogInvalidSignature(HEAD) LogErr("Function " HEAD ": Invalid function signature")
			case Lexer::TokenType::DEF: {
				argTokens.clear ();
				ReadTo (Lexer::TokenType::SEMICOLON, rawTokens, iToken, argTokens);
				QWord iSubToken = OpenCounter;
				if (IsUndefined (argTokens[iSubToken].m_Type)) {
					curToken = argTokens[iSubToken];
					String funName = std::move (std::get<String> (curToken.m_Val));
					FuncInfo funInfo;
					auto defineLocal = [&funInfo](const Lexer::Token& token) {
						funInfo.m_DefinedVars.push_back (std::get<String> (token.m_Val));
					};
					curToken = argTokens[++iSubToken];
					Lexer::Token paramHolder;
					SInt32 numArgs {};
					Boolean readyForDefine {};
					if (IsLParen (curToken.m_Type)) {
						iSubToken++;
						while (iSubToken < argTokens.size ()) {
							curToken = argTokens[iSubToken];
							if (IsComma (curToken.m_Type)) {
								if (readyForDefine) {
									numArgs++;
									defineLocal (paramHolder);
									readyForDefine = false;
								}
							}
							else if (IsRParen (curToken.m_Type)) {
								if (readyForDefine) {
									numArgs++;
									defineLocal (paramHolder);
								}
								break;
							}
							else {
								paramHolder = std::move (curToken);
								readyForDefine = true;
							}
							iSubToken++;
						}
					}
					funInfo.m_NumArgs = numArgs;
					readyForDefine = false;
					if (iSubToken < PrevWord (argTokens.size ())) {
						curToken = argTokens[++iSubToken];
						if (curToken.m_Type == Lexer::TokenType::DBLDOT) {
							iSubToken++;
							while (iSubToken < argTokens.size ()) {
								curToken = argTokens[iSubToken];
								if (IsComma (curToken.m_Type)) {
									defineLocal (paramHolder);
									readyForDefine = false;
								}
								else {
									paramHolder = std::move (curToken);
									readyForDefine = true;
								}
								iSubToken++;
							}

							if (readyForDefine) {
								defineLocal (paramHolder);
							}
						}
					}
					m_DefinedFuncs.emplace (funName, std::move (funInfo));
				}
				else {
					LogInvalidSignature ("definition");
				}
				break;
			}
			case Lexer::TokenType::IMP: {
				argTokens.clear ();
				if (iToken < PrevWord (rawTokens.size ()) && IsUndefined (rawTokens[NextWord (iToken)].m_Type)) {
					auto& funName = std::get<String> (rawTokens[++iToken].m_Val);

					auto funIt = m_DefinedFuncs.find (funName);
					if (funIt == m_DefinedFuncs.end ()) {
						std::cerr << "Function " << funName << " is not defined!" << std::endl;
						break;
					}

					if (IsLBrace (rawTokens[NextWord (iToken)].m_Type)) {
						auto& funInfo = funIt->second;
						ProcStatement (rawTokens, ++iToken, argTokens);
						QWord jmpOpId = m_Instructions.size ();
						Op (VM::JMP);
						funInfo.m_Address = static_cast<SInt64> (NextWord (jmpOpId));
						GenerateInstructionList (argTokens, funInfo.m_DefinedVars, false, lastWhileStmt);
						if (m_Instructions.back ().m_Op != VM::RETURN) {
							Push (Zero);
							Op (VM::RETURN);
						}
						m_Instructions[jmpOpId].m_Val = GetInstrEnd ();
					}
					else {
						LogInvalidSignature ("implementation");
					}
				}
				else {
					LogInvalidSignature ("implementation");
				}
				break;
			}
#undef LogInvalidSignature
			case Lexer::TokenType::IF: {
				Lexer::TokenList conditionExpr;
				ReadTo (Lexer::TokenType::LBRACE, rawTokens, iToken, conditionExpr);
				ProcExpression (conditionExpr, varSet, externalContext);
				QWord condJmpOpId = m_Instructions.size ();
				Op (VM::JMPC);

				Lexer::TokenList ifTokens;
				ProcStatement (rawTokens, iToken, ifTokens);
				GenerateInstructionList (ifTokens, varSet, externalContext, lastWhileStmt);

				QWord nextTokenId = NextWord (iToken);
				auto isNextElse = nextTokenId < rawTokens.size () && 
					rawTokens[NextWord (iToken)].m_Type Is Lexer::TokenType::ELSE;
				if (isNextElse) {
					iToken++;
					Op (VM::JMP);
				}

				m_Instructions[condJmpOpId].m_Val = GetInstrEnd ();

				if (isNextElse) {
					QWord endStatementJmpOpId = PrevWord (m_Instructions.size ());
					Lexer::TokenList elseTokens;
					if (IsLBrace (rawTokens[NextWord (iToken)].m_Type)) {
						ProcStatement (rawTokens, ++iToken, elseTokens);
						GenerateInstructionList (elseTokens, varSet, externalContext, lastWhileStmt);
						m_Instructions[endStatementJmpOpId].m_Val = GetInstrEnd (PrevWord ());
					}
					else {
						LogErr("Statement: Else statement should be defined with brackets!");
						return;
					}
				}
				break;
			}
			case Lexer::TokenType::WHILE: {
				Lexer::TokenList conditionExpr;

				QWord statementBeginId = m_Instructions.size ();
				ReadTo (Lexer::TokenType::LBRACE, rawTokens, iToken, conditionExpr);
				ProcExpression (conditionExpr, varSet, externalContext);

				QWord condJmpOpId = m_Instructions.size ();
				Op (VM::JMPC);

				Lexer::TokenList innerTokens;
				ProcStatement (rawTokens, iToken, innerTokens);

				WhileInfo stmtData {
					.m_Entry = statementBeginId
				};

				GenerateInstructionList (innerTokens, varSet, externalContext, stmtData);

				Op (VM::JMP);
				m_Instructions.back ().m_Val = VM::ValUtility::MemCpyQWord (&statementBeginId, true);

				const auto instrEnd = GetInstrEnd ();
				for (const auto& breakPt : stmtData.m_BreakPoints) {
					m_Instructions[breakPt].m_Val = instrEnd;
				}

				m_Instructions[condJmpOpId].m_Val = instrEnd;
				break;
			}
			case Lexer::TokenType::BREAK: {
				lastWhileStmt.m_BreakPoints.push_back (m_Instructions.size ());
				Op (VM::JMP);
				break;
			}
			case Lexer::TokenType::CONTINUE: {
				Op (VM::JMP);
				m_Instructions.back ().m_Val = VM::ValUtility::MemCpyQWord (&lastWhileStmt.m_Entry, true);
				break;
			}
		}
	}
}

SInt32 Compiler::ProcArgs (Lexer::TokenList& inTokens, SInt32& iToken, VarSet& varSet, Boolean externalContext, Lexer::TokenType opener) {
	const auto numTokens = static_cast<SInt64>(inTokens.size ());
	SInt32 numArgs = InvalidSignature;
	if (iToken < PrevWord (numTokens)) {
		auto& nextToken = inTokens[NextWord (iToken)];
		if (nextToken.m_Type Is opener) {
			numArgs = Zero;
			Lexer::TokenList argExprTokens;
			SInt32 numParen = OpenCounter;
			iToken += 2;
			while (numParen > Zero && iToken < numTokens) {
				nextToken = inTokens[iToken];
				if (IsLeftBracket (nextToken)) {
					numParen++;
				}
				else if (IsRightBracket (nextToken)) {
					if (--numParen <= Zero) {
						if (!argExprTokens.empty ()) {
							ProcExpression (argExprTokens, varSet, externalContext, true);
							argExprTokens.clear ();
							numArgs++;
						}
						break;
					}
				}
				else if (IsComma (nextToken.m_Type)) {
					if (numParen Is OpenCounter && !argExprTokens.empty ()) {
						ProcExpression (argExprTokens, varSet, externalContext, true);
						argExprTokens.clear ();
						numArgs++;
						iToken++;
						continue;
					}
				}
				argExprTokens.push_back (nextToken);
				iToken++;
			}
		}
		else return InvalidSignature;
	}
	return numArgs;
}

Undef Compiler::ReadTo (Lexer::TokenType itBreaker, Lexer::TokenList& inTokens, SInt32& iToken, Lexer::TokenList& outTokens) {
	if (iToken >= inTokens.size ()) {
		return;
	}

	while (iToken < inTokens.size ()) {
		auto& token = inTokens[iToken];
		if (token.m_Type Is itBreaker) {
			break;
		}
		outTokens.push_back (token);
		iToken++;
	}
}

Undef Compiler::ProcStatement (Lexer::TokenList& inTokens, SInt32& iToken, Lexer::TokenList& outTokens, Boolean stopOnBreaker) {
	iToken++;
	SInt32 numBrackets = OpenCounter;
	while (numBrackets > Zero && iToken < inTokens.size ()) {
		const auto& token = inTokens[iToken];
		if (IsLBrace (token.m_Type)) {
			numBrackets++;
		}
		else if (IsRBrace (token.m_Type)) {
			if (--numBrackets <= Zero && stopOnBreaker) {
				return;
			}
		}
		outTokens.push_back (token);
		iToken++;
	}
}

Undef Compiler::ProcExpression (Lexer::TokenList expressionTokens, VarSet& varSet, Boolean externalContext, Boolean innerExpr) {
	Stack<SInt32> rawTokenStack;
	const auto rAddressT = externalContext ? VM::RADDRESS_T : VM::LRADDRESS_T;
	const auto numTokens = static_cast<SInt64>(expressionTokens.size());
	for (SInt32 iToken = Zero; iToken < numTokens; iToken++) {
		auto&[m_Val, m_Type] = expressionTokens[iToken];
		switch (const auto tokenType = m_Type) {
			case Lexer::TokenType::LBRACE: {
				if (const SInt32 nArgs = ProcArgs (expressionTokens, --iToken, varSet, externalContext, Lexer::TokenType::LBRACE); nArgs > Zero) {
					Alloc (static_cast<UInt64> (nArgs));
				}
				break;
			}
			case Lexer::TokenType::STR: {
				if (const String* pStrValuePtr = std::get_if<String> (&m_Val)) {
					StringAlloc (*pStrValuePtr);
					Push (0x1);
					Op (VM::PIN);
				}
				break;
			}
			case Lexer::TokenType::CSTR: {
				if (const String* pStrValuePtr = std::get_if<String> (&m_Val)) {
					StringAlloc (*pStrValuePtr);
				}
				break;
			}
			case Lexer::TokenType::CHAR: {
				Push (std::get<SInt8> (m_Val));
				break;
			}
			case Lexer::TokenType::BYTE: {
				Push (std::get<UInt8> (m_Val));
				break;
			}
			case Lexer::TokenType::UINT16: {
				Push (std::get<UInt16> (m_Val));
				break;
			}
			case Lexer::TokenType::UINT32: {
				Push (std::get<UInt32> (m_Val));
				break;
			}
			case Lexer::TokenType::UINT64: {
				Push (std::get<UInt64> (m_Val));
				break;
			}
			case Lexer::TokenType::INT16: {
				Push (std::get<SInt16> (m_Val));
				break;
			}
			case Lexer::TokenType::INT32: {
				Push (std::get<SInt32> (m_Val));
				break;
			}
			case Lexer::TokenType::INT64: {
				Push (std::get<SInt64> (m_Val));
				break;
			}
			case Lexer::TokenType::FLOAT32: {
				Push (std::get<Dec32> (m_Val));
				break;
			}
			case Lexer::TokenType::FLOAT64: {
				Push (std::get<Dec64> (m_Val));
				break;
			}
			case Lexer::TokenType::UNDEFINED: {
				if (const String* pStrValuePtr = std::get_if<String> (&m_Val)) {
					if (InvokableOp invokableOp; TryGetDictionaryValue (InvokableOpMap, *pStrValuePtr, invokableOp)) {
						const SInt32 nArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext);
						if (invokableOp.m_PushNumArgs) {
							Push (nArgs);
						}
						Op (invokableOp.m_Operation);
						continue;
					}

					if (auto funIt = m_DefinedFuncs.find (*pStrValuePtr); funIt != m_DefinedFuncs.end ()) {
						auto& funInfo = funIt->second;

						const Boolean async = iToken < PrevWord (numTokens) && expressionTokens[NextWord (iToken)].m_Type Is Lexer::TokenType::ASYNC;
						if (async) {
							iToken++;
						}

						if (const SInt32 nArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext); IsValidID(nArgs)) {
							Push (funInfo.m_NumArgs);
							Push (static_cast<SInt32>(funInfo.m_DefinedVars.size ()));
							Call (funInfo, async);
						}
						else {
							funInfo.m_Calls.push_back (m_Instructions.size ());
							Push (0x0);
						}
						continue;
					}

					if (const SInt32 rAddressTest1 = Found (varSet, *pStrValuePtr); IsValidID(rAddressTest1) && !externalContext)
					{
						Push (rAddressTest1, rAddressT);
						continue;
					}

					if (const SInt32 rAddressTest2 = Found (m_DefinedVars, *pStrValuePtr); IsValidID(rAddressTest2)) {
						if (const SInt32 nArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext); nArgs != InvalidSignature) {
							Push (nArgs);
							if (iToken < PrevWord (numTokens) &&
								expressionTokens[NextWord (iToken)].m_Type Is Lexer::TokenType::DBLDOT) {
								iToken += 2;
								if (const String* pInnerStrValuePtr = std::get_if<String> (&expressionTokens[iToken].m_Val)) {
									if (auto refFunIt = m_DefinedFuncs.find (*pInnerStrValuePtr); refFunIt != m_DefinedFuncs.end ()) {
										auto& funInfo = refFunIt->second;
										Push (funInfo.m_DefinedVars.size ());
									}
								}
							}
							Push (rAddressTest2, VM::RADDRESS_T);
							m_Instructions.push_back (VM::Instruction {
								.m_Op  = VM::CALLADDR
							});
						}
						else {
							Push (rAddressTest2, VM::RADDRESS_T);
						}
						continue;
					}
					Push (static_cast<SInt32>(m_DefinedVars.size()), VM::RADDRESS_T);
					m_DefinedVars.push_back (*pStrValuePtr);
				}
				break;
			}
			default: {
				if (RangeInclude (tokenType, Lexer::TokenType::CPY, Lexer::TokenType::OR)) {
					while (!rawTokenStack.empty ()) {
						const auto  topId = rawTokenStack.top ();
						auto& topToken = expressionTokens[topId];

						if (!RangeInclude (topToken.m_Type, Lexer::TokenType::CPY, Lexer::TokenType::OR) ||
							Precedence (tokenType) > Precedence (topToken.m_Type))
							break;

						Op (topToken.m_Type);
						rawTokenStack.pop ();
					}
					rawTokenStack.push (iToken);
				}
				else {
					if (tokenType Is Lexer::TokenType::LSQRBRACKET) {
						if (iToken > Zero &&
							!RangeInclude (expressionTokens[PrevWord (iToken)].m_Type,
							Lexer::TokenType::CPY, Lexer::TokenType::OR)) {
							ProcArgs (expressionTokens, --iToken, varSet, externalContext, Lexer::TokenType::LSQRBRACKET);
							if (iToken Is PrevWord (numTokens) || expressionTokens[NextWord (iToken)].m_Type != Lexer::TokenType::WRITE) {
								Op (VM::MEMLOAD);
							}
							break;
						}
					}
					else if (IsLParen (tokenType)) {
						rawTokenStack.push (iToken);
					}
					else if (IsRParen (tokenType)) {
						while (!rawTokenStack.empty ()) {
							const auto topId = rawTokenStack.top ();
							auto& topToken = expressionTokens[topId];

							if (IsLParen (topToken.m_Type))
								break;

							Op (topToken.m_Type);
							rawTokenStack.pop ();
						}

						if (rawTokenStack.empty ()) {
							LogErr ("Mismatched parentheses!");
						}

						rawTokenStack.pop ();
					}
				}
			}
		}
	}

	while (!rawTokenStack.empty ()) {
		const auto topId = rawTokenStack.top ();
		auto& [m_Val, m_Type] = expressionTokens[topId];
		if (IsLParen (m_Type) || IsRParen (m_Type)) {
			LogErr ("Mismatched parentheses!");
		}
		Op (m_Type);
		rawTokenStack.pop ();
	}

	if (!m_Instructions.empty ()) {
		if (auto& [m_Op, m_Val] = m_Instructions[PrevWord (m_Instructions.size ())];
			!innerExpr && (m_Op Is VM::MEMCPY || m_Op Is VM::MEMSTORE)) {
			Op (VM::POP);
		}
	}
}

VM::ReadOnlyData Compiler::GetReadOnlyData () const {
	const Size nStrLiterals = m_StringLiterals.size ();
	VM::ReadOnlyData roData {
		.m_Memory = new Sym* [nStrLiterals],
		.m_Size = nStrLiterals
	};
	for (Size iLiteral = Zero; iLiteral < nStrLiterals; iLiteral++) {
		roData.m_Memory[iLiteral] = GetNewLPCStr (m_StringLiterals[iLiteral]);
	}
	return roData;
}

VM::Asm Compiler::GetAssemblyCode () const {
	const Size nInstr = m_Instructions.size ();
	auto* instr = new VM::Instruction[nInstr];
	std::memcpy (instr, m_Instructions.data (), nInstr * sizeof(VM::Instruction));
	return VM::Asm {
		.m_Memory = instr,
		.m_Size   = nInstr
	};
}

struct CKeywords final {
    constexpr static auto* Struct    = nameof(struct);
    constexpr static auto* Final     = nameof(final);
    constexpr static auto* Include   = nameof(#include);
    constexpr static auto* ConstExpr = nameof(constexpr);
    constexpr static auto* Static    = nameof(static);
    constexpr static auto* QWord     = nameof(UInt64);
};

String Compiler::MakeMetaHeader() {
    OStrStream metaBuffer;

#define Next() metaBuffer << LineSeparators::NextLine
#define Space() metaBuffer << "\n\n"
#define Include(HEADER) metaBuffer <<        \
    CKeywords::Include << ' ' <<             \
    Lexer::SymbolMap::QuoteChar << HEADER << \
    Lexer::SymbolMap::QuoteChar
#define MakeStruct(NAME) metaBuffer << \
    CKeywords::Struct << ' ' <<        \
    NAME << ' ' <<                     \
    CKeywords::Final << " {"
#define CloseStruct() metaBuffer << "};"
#define Indent() metaBuffer << "    "
#define MakeWord(NAME, VALUE) metaBuffer << \
    CKeywords::ConstExpr << ' ' <<   \
    CKeywords::Static << ' ' <<      \
    CKeywords::QWord << ' ' <<       \
    NAME << " = " << VALUE << ';'

    Include("Utility.h");
    Space();

    MakeStruct("ExtractedVariablesMeta");
    Next();

    for (UInt64 i = 0; i < m_DefinedVars.size(); ++i) {
        Indent();
        MakeWord(m_DefinedVars[i], i);
        Next();
    }

    CloseStruct();
    Space();

    MakeStruct("ExtractedFunctionsMeta");
    Next();

    for (auto& fun : m_DefinedFuncs) {
        Indent();
        MakeWord(fun.first, fun.second.m_Address);
        Next();
    }

    CloseStruct();

    return metaBuffer.str();

#undef Include
#undef MakeStruct
#undef CloseStruct
#undef Indent
#undef MakeWord
}
