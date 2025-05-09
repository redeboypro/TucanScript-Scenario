#include "..\Compiler.h"
#include "..\BinaryBuilder.h"

#define STACK_SIZE     512ULL
#define HEAP_SIZE      512ULL
#define CALL_MAX_DEPTH 1024

TucanScript::ProgramExitCode_t main(TucanScript::Undef) {
	//auto lexer = new TucanScript::Lexer::Tokenizer ();

	//auto compiler = new TucanScript::Compiler ();
	//compiler->GenerateInstructionList (lexer->Tokenize (TucanScript::ReadFileContent ("Test.ts")));
	//compiler->LogInstr ();
	//TucanScript::VM::Asm assembly = compiler->GetAssemblyCode ();
	//TucanScript::VM::ReadOnlyData roData = compiler->GetReadOnlyData ();

	//delete lexer;
	//delete compiler;

	//TucanScript::Binary::BinaryBuilder::Build (roData, assembly, "Test.tbin");

	//delete assembly.m_Memory;
	//TucanScript::VM::DeleteROData (roData);
	//return Zero;

	TucanScript::VM::ReadOnlyData roData {};
	TucanScript::VM::Asm asm_ {};
	TucanScript::Binary::BinaryBuilder::Decompose ("D:\\TucanScript\\Tests\\Test.tbin", asm_, roData);

	auto staticDealloc = new TucanScript::VM::UnsafeDeallocator ();
	staticDealloc->PutReadOnlyData (roData);

	auto vm = new TucanScript::VM::VirtualMachine (
		STACK_SIZE, 
		HEAP_SIZE, 
		CALL_MAX_DEPTH,
		std::move(asm_),
		staticDealloc);

	vm->Run ();

	delete vm;
	return Zero;
}