#ifndef UTILITY_H
#define UTILITY_H

#if _WIN64
	#include <windows.h>
#else
	#include <dlfcn.h>
	#define __stdcall
#endif

#include <chrono>
#include <iostream>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <type_traits>
#include <stack>
#include <deque>
#include <fstream>
#include <unordered_set>
#include <mutex>

namespace TucanScript {
#pragma region [Macros & API Definitions]
	#define nameof(x) #x
	#ifndef ExternC
	#define ExternC extern "C"
	#endif
	#define TucanAPI __declspec(dllexport)
#pragma endregion

#pragma region [Math and Shortcuts]
	#define Zero 0
	#define Clamp(ANGLE, MIN, MAX) ANGLE < MIN ? MIN : ANGLE > MAX ? MAX : ANGLE
	#define NextWord(WORD) (WORD + 1)
	#define PrevWord(WORD) (WORD - 1)
	#define Is ==
#pragma endregion

#pragma region [Diagnostics]
	#define Log(MSG)      std::cout << MSG << std::endl
	#define LogErr(MSG)   std::cerr << MSG << std::endl
	#define Notify(MSG)   MessageBoxW(nullptr, MSG, L"Log", MB_OK | MB_ICONEXCLAMATION)
	#define Terminate()   exit(EXIT_FAILURE)
	#define InvalidID     -1
	#define IsValidID(ID) (ID != InvalidID)
#pragma endregion

#pragma region [Basic Type Aliases]
	typedef char Sym;

	typedef signed char      SInt8;
	typedef signed short     SInt16;
	typedef signed int       SInt32;
	typedef signed long long SInt64;

	typedef unsigned char      UInt8;
	typedef unsigned short     UInt16;
	typedef unsigned int       UInt32;
	typedef unsigned long long UInt64;

	typedef float  Dec32;
	typedef double Dec64;

	typedef void Undef;
	typedef bool Boolean;

	struct LineSeparators final {
		constexpr static SInt8 NextLine    = '\n';
		constexpr static SInt8 CarriageRet = '\r';
	};
#pragma endregion

#pragma region [Common System Types and Utilities]
	typedef UInt64 Size;

	typedef SInt32 ProgramExitCode_t;

	typedef UInt8 Bitmask;

	typedef UInt64 QWORD;
	typedef UInt32 DWORD;

	template <typename T>
	struct MemoryView final {
		T*   m_Memory;
		Size m_Size;
	};

	template<typename A_T, typename B_T>
	constexpr Boolean SizeEquals () {
		return sizeof (A_T) == sizeof (B_T);
	}
#pragma endregion

#pragma region [STL Aliases and Utilities]
	typedef std::string String;
	typedef std::uintptr_t IntPtr;

	typedef std::istringstream IStrStream;
	typedef std::stringstream  StrStream;

	template <typename T>
	using Stack = std::stack<T>;

	template <typename T>
	using SharedPtr = std::shared_ptr<T>;

	template <typename T>
	using Deque = std::deque<T>;

	template <typename KEY, typename VALUE>
	using Dictionary = std::unordered_map<KEY, VALUE>;

	template <typename KTY, typename HASHER>
	using UnorderedSet = std::unordered_set<KTY, HASHER>;

	template <typename ...T>
	using Variant = std::variant<T...>;

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T>
	using InitializerList = std::initializer_list<T>;

	using IFileStream = std::ifstream;
	using OFileStream = std::ofstream;

	using Mutex = std::mutex;

	template<typename T>
	Boolean TryParse (const String& str, T& value) {
		IStrStream iStrStream (str);
		iStrStream >> value;
		return !iStrStream.fail ();
	}

	template<typename KEY, typename VALUE>
	Boolean TryGetDictionaryValue (const Dictionary<KEY, VALUE>& dictionary, const KEY& key, VALUE& value) {
		auto it = dictionary.find (key);
		if (it != dictionary.end ()) {
			value = it->second;
			return true;
		}
		return false;
	}

	template <typename TYPE>
	inline SInt32 Found (const Vector<TYPE>& set, const TYPE& value) {
		auto it = std::find (set.begin (), set.end (), value);
		if (it != set.end ()) {
			return static_cast<SInt32>(std::distance (set.begin (), it));
		}
		return -1;
	}

	inline String ReadFileContent (const String& filePath) {
		IFileStream file (filePath);

		if (!file.is_open ()) {
			LogErr ("Error opening file: " << filePath);
			return "";
		}

		StrStream buffer;
		buffer << file.rdbuf ();
		return buffer.str ();
	}

	inline Sym* GetNewLPCStr (const String& str) {
		Sym* buffer = new Sym[NextWord (str.size ())];
		std::strcpy (buffer, str.c_str ());
		return buffer;
	}
#pragma endregion

#pragma region [Lib Loader]
	inline Undef* LoadLib (const Sym* libFileName) {
	#if _WIN64
		return LoadLibraryA (libFileName);
	#else
		return dlopen (libFileName, RTLD_LAZY);
	#endif
	}

	inline Undef* GetModule (const Sym* libFileName) {
	#if _WIN64
		return GetModuleHandleA (libFileName);
	#else
		return dlopen (libFileName, RTLD_NOW | RTLD_NOLOAD);
	#endif
	}

	inline Undef* ProcAddr (Undef* handle, const Sym* symbolName) {
	#if _WIN64
		return (Undef*) (GetProcAddress ((HMODULE) handle, symbolName));
	#else
		return dlsym (handle, symbolName);
	#endif
	}

	inline void FreeLib (Undef* handle) {
	#if _WIN64
		FreeLibrary ((HMODULE) handle);
	#else
		dlclose (handle);
	#endif
	}
#pragma endregion
}

#endif