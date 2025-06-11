#ifndef LEXER_H
#define LEXER_H

#include "Utility.h"

namespace TucanScript::Lexer {
	struct Token;

	using TokenList = std::vector<Token>;
	using TokenVal  = Variant<
		String,
		SInt8, SInt16, SInt32, SInt64,
		UInt8, UInt16, UInt32, UInt64,
		Dec32, Dec64>;

	struct SymbolMap final {
		constexpr static SInt8 MinusChar       = '-';
		constexpr static SInt8 CommentChar     = '#';
		constexpr static SInt8 QuoteChar       = '"';
		constexpr static SInt8 SlashChar       = '\\';
		constexpr static SInt8 NextLineSpecial = 'n';
	};

	enum class TokenType : SInt32 {
		//Anonymous
		NONE,
		UNDEFINED,

		//Data types
		STR,
		CHAR,
		BYTE,
		UINT16,
		UINT32,
		UINT64,
		INT16,
		INT32,
		INT64,
		FLOAT32,
		FLOAT64,

		//Operators
		CPY,
		CMPE,
		CMPG,
		CMPL,
		CMPGE,
		CMPLE,
		NOT,
		CMPNE,

		WRITE,
		READ,

		PLUS,
		MINUS,
		MUL,
		DIV,

		AND,
		OR,

		//Statements
		IF,
		ELSE,
		WHILE,

		DEF,
		IMP,

		//Separators
		SEMICOLON,
		COMMA,
		DBLDOT,

		//Brackets
		LPAREN,
		RPAREN,

		LBRACE,
		RBRACE,

		LSQRBRACKET,
		RSQRBRACKET,

		//Statement goto
		BREAK,
		CONTINUE,

		//Additional
		ASYNC,
		INCLUDE,

		CSTR
	};

	struct Token final {
		TokenVal  m_Val;
		TokenType m_Type { TokenType::NONE };
	};

	static const Dictionary<Sym, TokenType> ReservedSingleCharMap {
		{ '=', TokenType::CPY },
		{ '*', TokenType::MUL },
		{ '/', TokenType::DIV },
		{ '+', TokenType::PLUS },
		{ '-', TokenType::MINUS },
		{ '>', TokenType::CMPG },
		{ '<', TokenType::CMPL },
		{ '(', TokenType::LPAREN },
		{ ')', TokenType::RPAREN },
		{ '[', TokenType::LSQRBRACKET },
		{ ']', TokenType::RSQRBRACKET },
		{ '{', TokenType::LBRACE },
		{ '}', TokenType::RBRACE },
		{ ';', TokenType::SEMICOLON },
		{ ',', TokenType::COMMA },
		{ '!', TokenType::NOT },
		{ ':', TokenType::DBLDOT }
	};

	static const Dictionary<String, TokenType> ReservedWordMap = {
		{ "and",      TokenType::AND },
		{ "or",       TokenType::OR },
		{ "if",       TokenType::IF },
		{ "else",     TokenType::ELSE },
		{ "while",    TokenType::WHILE },
		{ "def",      TokenType::DEF },
		{ "imp",      TokenType::IMP },
		{ "break",    TokenType::BREAK },
		{ "continue", TokenType::CONTINUE },
		{ "include",  TokenType::INCLUDE },
		{ "async",    TokenType::ASYNC },
		{ "cstr",     TokenType::CSTR }
	};

	class Tokenizer final {
		inline Boolean IsDigitOrMinus (const Sym sym) {
			return std::isdigit (sym) || sym Is SymbolMap::MinusChar;
		}

		template<TokenType CMPT>
		inline Boolean BothAre (const TokenType t1, const TokenType t2) {
			return t1 Is CMPT and t2 Is CMPT;
		}
		
		template<typename T>
		Undef ParseAndApplyNumeric (const String& tokenStr, TokenVal& tokenValue) {
			T result {};
			if (TryParse<T> (tokenStr, result)) {
				tokenValue = result;
			}
			else {
				LogErr ("Cannot parse not numeric token!");
			}
		}

		Boolean StartsWithDigitOrMinus (const String& str);
		Undef ProcNumericSuffix (Sym source, TokenType& type);
		Token CreateToken (const TokenVal& value, const TokenType type);

	public:
		Boolean IsTokenReservedSingleChar (Sym source, TokenType& type);
		Boolean IsTokenReservedWord (const String& source, TokenType& type);

		TokenList Tokenize (const String& source);
		TokenList ProcessIncludeDirectories (const TokenList& tokens, String includeSearchDir);
	};
}

#endif