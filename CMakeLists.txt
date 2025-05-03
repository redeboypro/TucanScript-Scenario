cmake_minimum_required (VERSION 3.8)
project ("TucanScript")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (TucanScriptTest
			 "TucanScript/Test/main.cpp"
			 TucanScript/Utility.h
			 TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp 
			 TucanScript/Lexer.h TucanScript/Lexer.cpp
			 TucanScript/Compiler.h TucanScript/Compiler.cpp "TucanScript/BinaryBuilder.h" "TucanScript/BinaryBuilder.cpp")

add_library (TucanScript SHARED
			 TucanScript/Utility.h
			 TucanScript/VirtualMachine.h TucanScript/VirtualMachine.cpp 
			 TucanScript/Lexer.h TucanScript/Lexer.cpp 
			 TucanScript/Compiler.h TucanScript/Compiler.cpp "TucanScript/BinaryBuilder.h" "TucanScript/BinaryBuilder.cpp")