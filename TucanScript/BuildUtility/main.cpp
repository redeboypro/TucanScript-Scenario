#include "../Compiler.h"
#include "../BinaryBuilder.h"

using namespace TucanScript;

#define ExtSeparator "."
#define BinaryExt    ".tbin"

ProgramExitCode_t main (SInt32 nArgs, Sym* args[]) {
	auto lexer = new Lexer::Tokenizer ();

	if (nArgs <= 1) {
		return nArgs;
	}

	String entryFilePath  = args[1];
	String binaryFilePath = entryFilePath.substr (Zero, entryFilePath.find_last_of (ExtSeparator)).append (BinaryExt);

	Log ("Output binary: " << binaryFilePath);

	String includeDir;
	if (nArgs > 2) {
		includeDir = args[2];
	}

	auto compiler = new Compiler ();
	compiler->GenerateInstructionList (
		lexer->ProcessIncludeDirectories (
			lexer->Tokenize (
				ReadFileContent (entryFilePath)
			), 
			includeDir
		)
	);

	compiler->LogInstr ();
	VM::Asm assembly = compiler->GetAssemblyCode ();
	VM::ReadOnlyData roData = compiler->GetReadOnlyData ();

	delete lexer;
	delete compiler;

	Binary::BinaryBuilder::Build (roData, assembly, binaryFilePath);

	delete assembly.m_Memory;
	VM::DeleteROData (roData);
	return Zero;
}