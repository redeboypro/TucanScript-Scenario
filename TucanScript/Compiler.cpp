#include "Compiler.h"

using namespace TucanScript;

#define LogOpenStream(DATA)  std::cout << (DATA)
#define Log2OpenStream(A, B) std::cout << "(" << (A) << "; " << (B) << ")"
Undef TucanScript::Compiler::LogInstr () {
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

Undef TucanScript::Compiler::GenerateInstructionList (Lexer::TokenList rawTokens, VarSet& varSet, Boolean externalContext, WhileInfo& lastWhileStmt) {
	Lexer::TokenList argTokens;
	for (SInt32 iToken = 0; iToken < rawTokens.size (); iToken++) {
		auto& curToken = rawTokens[iToken];
		switch (curToken.m_Type) {
			case Lexer::TokenType::UNDEFINED: {
				argTokens.clear ();
				ReadTo (Lexer::TokenType::SEMICOLON, rawTokens, iToken, argTokens);
				ProcExpression (argTokens, varSet, externalContext);
				break;
			}
#define LogInvalidSignature(HEAD) LogErr("Function " HEAD ": Invalid function signature")
			case Lexer::TokenType::DEF: {
				argTokens.clear ();
				ReadTo (Lexer::TokenType::SEMICOLON, rawTokens, iToken, argTokens);
				QWORD iSubToken = OpenCounter;
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
						QWORD jmpOpId = m_Instructions.size ();
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
				QWORD condJmpOpId = m_Instructions.size ();
				Op (VM::JMPC);

				Lexer::TokenList ifTokens;
				ProcStatement (rawTokens, iToken, ifTokens);
				GenerateInstructionList (ifTokens, varSet, externalContext, lastWhileStmt);

				QWORD nextTokenId = NextWord (iToken);
				auto isNextElse = nextTokenId < rawTokens.size () && 
					rawTokens[NextWord (iToken)].m_Type Is Lexer::TokenType::ELSE;
				if (isNextElse) {
					iToken++;
					Op (VM::JMP);
				}

				m_Instructions[condJmpOpId].m_Val = GetInstrEnd ();

				if (isNextElse) {
					QWORD endStatementJmpOpId = PrevWord (m_Instructions.size ());
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

				QWORD statementBeginId = m_Instructions.size ();
				ReadTo (Lexer::TokenType::LBRACE, rawTokens, iToken, conditionExpr);
				ProcExpression (conditionExpr, varSet, externalContext);

				QWORD condJmpOpId = m_Instructions.size ();
				Op (VM::JMPC);

				Lexer::TokenList innerTokens;
				ProcStatement (rawTokens, iToken, innerTokens);

				WhileInfo stmtData {
					.m_Entry = statementBeginId
				};

				GenerateInstructionList (innerTokens, varSet, externalContext, stmtData);

				Op (VM::JMP);
				m_Instructions.back ().m_Val = VM::ValUtility::_QWORD_signed_raw (&statementBeginId, true);

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
				m_Instructions.back ().m_Val = VM::ValUtility::_QWORD_signed_raw (&lastWhileStmt.m_Entry, true);
				break;
			}
		}
	}
}

SInt32 TucanScript::Compiler::ProcArgs (Lexer::TokenList& inTokens, SInt32& iToken, VarSet& varSet, Boolean externalContext, Lexer::TokenType opener) {
	const SInt64 numTokens = static_cast<SInt64>(inTokens.size ());
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
					if ((--numParen) <= Zero) {
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

Undef TucanScript::Compiler::ReadTo (Lexer::TokenType itBreaker, Lexer::TokenList& inTokens, SInt32& iToken, Lexer::TokenList& outTokens) {
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

Undef TucanScript::Compiler::ProcStatement (Lexer::TokenList& inTokens, SInt32& iToken, Lexer::TokenList& outTokens, Boolean stopOnBreaker) {
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

Undef TucanScript::Compiler::ProcExpression (Lexer::TokenList expressionTokens, VarSet& varSet, Boolean externalContext, Boolean innerExpr) {
	Stack<SInt32> rawTokenStack;
	auto rAddressT = externalContext ? VM::RADDRESS_T : VM::LRADDRESS_T;
	const Size numTokens = expressionTokens.size ();
	for (SInt32 iToken = Zero; iToken < numTokens; iToken++) {
		auto& token = expressionTokens[iToken];
		auto  tokenType = token.m_Type;
		switch (tokenType) {
			case Lexer::TokenType::LBRACE: {
				SInt32 numArgs = ProcArgs (expressionTokens, --iToken, varSet, externalContext, Lexer::TokenType::LBRACE);
				if (numArgs > Zero) {
					Alloc (static_cast<UInt64> (numArgs));
				}
				break;
			}
			case Lexer::TokenType::STR: {
				if (const String* strValuePtr = std::get_if<String> (&token.m_Val)) {
					StringAlloc (*strValuePtr);
				}
				break;
			}
			case Lexer::TokenType::CHAR: {
				Push (std::get<SInt8> (token.m_Val));
				break;
			}
			case Lexer::TokenType::BYTE: {
				Push (std::get<UInt8> (token.m_Val));
				break;
			}
			case Lexer::TokenType::UINT16: {
				Push (std::get<UInt16> (token.m_Val));
				break;
			}
			case Lexer::TokenType::UINT32: {
				Push (std::get<UInt32> (token.m_Val));
				break;
			}
			case Lexer::TokenType::UINT64: {
				Push (std::get<UInt64> (token.m_Val));
				break;
			}
			case Lexer::TokenType::INT16: {
				Push (std::get<SInt16> (token.m_Val));
				break;
			}
			case Lexer::TokenType::INT32: {
				Push (std::get<SInt32> (token.m_Val));
				break;
			}
			case Lexer::TokenType::INT64: {
				Push (std::get<SInt64> (token.m_Val));
				break;
			}
			case Lexer::TokenType::FLOAT32: {
				Push (std::get<Dec32> (token.m_Val));
				break;
			}
			case Lexer::TokenType::FLOAT64: {
				Push (std::get<Dec64> (token.m_Val));
				break;
			}
			case Lexer::TokenType::UNDEFINED: {
				if (const String* strValuePtr = std::get_if<String> (&token.m_Val)) {
					InvokableOp invokableOp;
					if (TryGetDictionaryValue (InvokableOpMap, *strValuePtr, invokableOp)) {
						SInt32 numArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext);
						if (invokableOp.m_PushNumArgs) {
							Push (numArgs);
						}
						Op (invokableOp.m_Operation);
						continue;
					}

					auto funIt = m_DefinedFuncs.find (*strValuePtr);
					if (funIt != m_DefinedFuncs.end ()) {
						auto& funInfo = funIt->second;

						Boolean async = iToken < PrevWord (numTokens) && expressionTokens[NextWord (iToken)].m_Type Is Lexer::TokenType::ASYNC;
						if (async) {
							iToken++;
						}

						SInt32 nArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext);
						Log (nArgs);
						if (IsValidID (nArgs)) {
							Push (funInfo.m_NumArgs);
							Push (static_cast<SInt32>(funInfo.m_DefinedVars.size ()));
							Call (funInfo, async);
						}
						else {
							LogErr ("Invalid call signature for " << *strValuePtr);
						}
						continue;
					}

					SInt32 rAddressTest1 = Found (varSet, *strValuePtr);
					if (IsValidID (rAddressTest1) && !externalContext)
					{
						Push (rAddressTest1, rAddressT);
						continue;
					}

					SInt32 rAddressTest2 = Found (m_DefinedVars, *strValuePtr);
					if (IsValidID (rAddressTest2)) {
						SInt32 numArgs = ProcArgs (expressionTokens, iToken, varSet, externalContext);
						Push (rAddressTest2, VM::RADDRESS_T);
						if (numArgs != InvalidSignature) {
							m_Instructions.push_back (VM::Instruction {
								.m_Op  = VM::DOEXCALL,
								.m_Val = VM::ValUtility::_DWORD_signed_raw(&numArgs, false)
							});
						}
						continue;
					}
					Push (m_DefinedVars.size (), VM::RADDRESS_T);
					m_DefinedVars.push_back (*strValuePtr);
				}
				break;
			}
			default: {
				if (RangeInclude (tokenType, Lexer::TokenType::CPY, Lexer::TokenType::OR)) {
					while (!rawTokenStack.empty ()) {
						auto  topId = rawTokenStack.top ();
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
							auto  topId = rawTokenStack.top ();
							auto& topToken = expressionTokens[topId];

							if (IsLParen (topToken.m_Type))
								break;

							Op (topToken.m_Type);
							rawTokenStack.pop ();
						}

						if (rawTokenStack.empty ()) {
							std::cerr << "Mismatched parentheses!" << std::endl;
						}

						rawTokenStack.pop ();
					}
				}
			}
		}
	}

	while (!rawTokenStack.empty ()) {
		auto  topId = rawTokenStack.top ();
		auto& topToken = expressionTokens[topId];
		if (IsLParen (topToken.m_Type) || IsRParen (topToken.m_Type)) {
			std::cerr << "Mismatched parentheses!" << std::endl;
		}
		Op (topToken.m_Type);
		rawTokenStack.pop ();
	}

	if (!m_Instructions.empty ()) {
		auto& lastOp = m_Instructions[PrevWord (m_Instructions.size ())];
		if (!innerExpr) {
			if ((lastOp.m_Op Is VM::MEMCPY || lastOp.m_Op Is VM::MEMSTORE)) {
				Op (VM::POP);
			}
		}
	}
}

VM::ReadOnlyData TucanScript::Compiler::GetReadOnlyData () {
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

VM::Asm TucanScript::Compiler::GetAssemblyCode () {
	const Size nInstr = m_Instructions.size ();
	VM::Instruction* instr = new VM::Instruction[nInstr];
	std::memcpy (instr, m_Instructions.data (), nInstr * sizeof(VM::Instruction));
	return VM::Asm {
		.m_Memory = instr,
		.m_Size   = nInstr
	};
}