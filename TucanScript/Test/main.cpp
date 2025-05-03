#include "..\Compiler.h"

#define STACK_SIZE     512ULL
#define HEAP_SIZE      512ULL
#define CALL_MAX_DEPTH 1024

TucanScript::ProgramExitCode_t main(TucanScript::Undef) {
	auto lexer = new TucanScript::Lexer::Tokenizer ();

	auto compiler = new TucanScript::Compiler ();
	compiler->GenerateInstructionList (lexer->Tokenize (TucanScript::ReadFileContent ("Test.ts")));
	compiler->LogInstr ();
	TucanScript::VM::Asm assembly = compiler->GetAssemblyCode ();

	delete lexer;
	delete compiler;

	auto staticDealloc = new TucanScript::VM::UnsafeDeallocator ();
	auto vm = new TucanScript::VM::VirtualMachine (
		STACK_SIZE, 
		HEAP_SIZE, 
		CALL_MAX_DEPTH,
		std::move(assembly),
		staticDealloc);

	vm->Run ();

	delete vm;
}