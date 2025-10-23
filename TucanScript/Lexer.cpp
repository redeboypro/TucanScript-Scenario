#include "Lexer.h"

using namespace TucanScript;

#define IsUnsignedSufix(CHAR) ((CHAR) == 'u' or (CHAR) == 'U')
#define IsLongSufix(CHAR)     ((CHAR) == 'l' or (CHAR) == 'L')
#define IsFloatSufix(CHAR)    ((CHAR) == 'f' or (CHAR) == 'F')
#define IsDoubleSufix(CHAR)   ((CHAR) == 'd' or (CHAR) == 'D')
#define IsDot(CHAR)           ((CHAR) == '.')

Boolean TucanScript::Lexer::Tokenizer::StartsWithDigitOrMinus (const String& str) {
	if (str.empty ()) {
		return false;
	}
	return IsDigitOrMinus (str[Zero]);
}

Undef TucanScript::Lexer::Tokenizer::ProcNumericSuffix (Sym source, TokenType& type) {
	if (IsFloatSufix (source)) {
		type = TokenType::FLOAT32;
	}
	else if (IsDoubleSufix (source)) {
		type = TokenType::FLOAT64;
	}
	else if (IsLongSufix (source)) {
		if (type Is TokenType::UINT32) {
			type = TokenType::UINT64;
		}
		else {
			type = TokenType::INT64;
		}
	}
	else if (IsUnsignedSufix (source)) {
		type = TokenType::UINT32;
	}
}

Lexer::Token TucanScript::Lexer::Tokenizer::CreateToken (const TokenVal& value, const TokenType type) {
	Token entryToken {
		.m_Val  = value,
		.m_Type = type
	};
	
	if (type Is TokenType::STR || type Is TokenType::CSTR) {
		return entryToken;
	}

	if (const String* variantStr = std::get_if<String> (&value)) {
		static TokenType givenType;
		auto& tokenStr = *variantStr;

		if (IsTokenReservedWord (tokenStr, givenType)) {
			return Token {
				.m_Val  = Zero,
				.m_Type = givenType
			};
		}

		Boolean isDecimal = false;
		TokenType numericType = TokenType::INT32;
		String unparsedToken;
		if (StartsWithDigitOrMinus (tokenStr)) {
			for (auto curChar : tokenStr) {
				if (IsDigitOrMinus (curChar)) {
					unparsedToken += curChar;
				}
				else if (IsDot (curChar) and !isDecimal) {
					unparsedToken += curChar;
					numericType = TokenType::FLOAT64;
					isDecimal = true;
				}
				else {
					ProcNumericSuffix (curChar, numericType);
				}
			}

			TokenVal tokenValue;
			switch (numericType) {
				case TokenType::INT32: {
					ParseAndApplyNumeric<SInt32> (unparsedToken, tokenValue);
					break;
				}
				case TokenType::INT64: {
					ParseAndApplyNumeric<SInt64> (unparsedToken, tokenValue);
					break;
				}
				case TokenType::UINT32: {
					ParseAndApplyNumeric<UInt32> (unparsedToken, tokenValue);
					break;
				}
				case TokenType::UINT64: {
					ParseAndApplyNumeric<UInt64> (unparsedToken, tokenValue);
					break;
				}
				case TokenType::FLOAT32: {
					ParseAndApplyNumeric<Dec32> (unparsedToken, tokenValue);
					break;
				}
				case TokenType::FLOAT64: {
					ParseAndApplyNumeric<Dec64> (unparsedToken, tokenValue);
					break;
				}
			}

			return Token {
				.m_Val  = tokenValue,
				.m_Type = numericType
			};
		}
	}
	return entryToken;
}

Boolean TucanScript::Lexer::Tokenizer::IsTokenReservedSingleChar (Sym source, TokenType& type) {
	return TryGetDictionaryValue (ReservedSingleCharMap, source, type);
}

Boolean TucanScript::Lexer::Tokenizer::IsTokenReservedWord (const String& source, TokenType& type) {
	return TryGetDictionaryValue (ReservedWordMap, source, type);
}

Lexer::TokenList TucanScript::Lexer::Tokenizer::Tokenize (const String& source) {
	String tokenStr;

	auto curType = TokenType::NONE;
	auto nextType = TokenType::NONE;

	TokenList rawTokens;
	auto tryCreateTokenFromWord = [this, &rawTokens](String& tokenStr) {
		if (!tokenStr.empty ()) {
			rawTokens.push_back (std::move (CreateToken (tokenStr, TokenType::UNDEFINED)));
			tokenStr.clear ();
		}
	};

	for (UInt64 iChar = Zero; iChar < source.size (); ++iChar) {
		Sym curChar = source[iChar];

		if (curChar Is SymbolMap::CommentChar) {
			do {
				curChar = source[++iChar];
			} while (iChar < source.size () && 
					 curChar != LineSeparators::NextLine && 
					 curChar != LineSeparators::CarriageRet);
			continue;
		}

		if (std::isspace (curChar)) {
			tryCreateTokenFromWord (tokenStr);
			continue;
		}

		if (IsTokenReservedSingleChar (curChar, curType)) {
			tryCreateTokenFromWord (tokenStr);

			if (iChar < source.size () - 1) {
				auto nextChar = source[NextWord(iChar)];
				if (IsTokenReservedSingleChar (nextChar, nextType)) {
					if (BothAre<TokenType::CMPL> (curType, nextType)) {
						rawTokens.push_back (std::move (CreateToken (Zero, TokenType::WRITE)));
						iChar++;
						continue;
					}

					else if (BothAre<TokenType::CMPG> (curType, nextType)) {
						rawTokens.push_back (std::move (CreateToken (Zero, TokenType::READ)));
						iChar++;
						continue;
					}

					if (nextType Is TokenType::CPY) {
						switch (curType) {
							case TokenType::CPY:
							rawTokens.push_back (std::move (CreateToken (Zero, TokenType::CMPE)));
							break;
							case TokenType::NOT:
							rawTokens.push_back (std::move (CreateToken (Zero, TokenType::CMPNE)));
							break;
							case TokenType::CMPG:
							rawTokens.push_back (std::move (CreateToken (Zero, TokenType::CMPGE)));
							break;
							case TokenType::CMPL:
							rawTokens.push_back (std::move (CreateToken (Zero, TokenType::CMPLE)));
							break;
						}
						iChar++;
						continue;
					}
				}

				if (curChar Is SymbolMap::MinusChar && std::isdigit (nextChar)) {
					tokenStr += curChar;
					continue;
				}
			}
			rawTokens.push_back (std::move (CreateToken (Zero, curType)));
			continue;
		}

		if (curChar Is SymbolMap::QuoteChar) {
			iChar++;
			auto strValueUnit = source[iChar];
			while (strValueUnit != SymbolMap::QuoteChar) {
				if (strValueUnit Is SymbolMap::SlashChar) {
					switch (source[++iChar]) {
						case SymbolMap::SlashChar:
						tokenStr += SymbolMap::SlashChar;
						break;
						case SymbolMap::QuoteChar:
						tokenStr += SymbolMap::QuoteChar;
						break;
						case SymbolMap::NextLineSpecial:
						tokenStr += LineSeparators::NextLine;
						break;
					}
				}
				else {
					tokenStr += strValueUnit;
				}
				strValueUnit = source[++iChar];
			}

			TokenType strTokenType = TokenType::STR;

			constexpr static auto R_PREFIX = 'r';

			UInt64 iPostfixChar = NextWord (iChar);
			if (iPostfixChar < source.size () && source[iPostfixChar] == R_PREFIX) {
				strTokenType = TokenType::CSTR;
				++iChar;
			}

			rawTokens.push_back (std::move (CreateToken (tokenStr, strTokenType)));
			tokenStr.clear ();
			continue;
		}

		if (!std::iscntrl (curChar)) {
			tokenStr += curChar;
		}
	}

	tryCreateTokenFromWord (tokenStr);
	return rawTokens;
}

Lexer::TokenList TucanScript::Lexer::Tokenizer::ProcessIncludeDirectories (const TokenList& tokens, String includeSearchDir) {
	TokenList processedToken;

    if (!includeSearchDir.empty())
    {
        while (includeSearchDir.back() == '/' || includeSearchDir.back() == '\\') {
            includeSearchDir.pop_back();
        }

        includeSearchDir = (std::filesystem::path(includeSearchDir) / "").string();
    }

	for (Size iToken = Zero; iToken < tokens.size (); iToken++) {
		auto& token = tokens[iToken];
		if (token.m_Type Is TokenType::INCLUDE) {
			const auto* strValuePtr = std::get_if<String> (&tokens[++iToken].m_Val);
			if (strValuePtr && iToken < PrevWord (tokens.size ())) {
                auto fullPath = std::filesystem::path(includeSearchDir) / *strValuePtr;

                TokenList includeTokens = ProcessIncludeDirectories(
                        Tokenize(ReadFileContent(fullPath.string())),
                        includeSearchDir
                );

				processedToken.insert (processedToken.end (), includeTokens.begin (), includeTokens.end ());
			}
			else {
				iToken--;
				LogErr ("Invalid include format!");
			}
		}
		else {
			processedToken.push_back (token);
		}
	}

	return processedToken;
}
