#include "../Compiler.h"
#include "../BinaryBuilder.h"

using namespace TucanScript;

#define BinaryExt     ".tbin"
#define MetaHeaderExt ".h"

ProgramExitCode_t main (SInt32 nArgs, Sym* args[]) {
	if (nArgs <= 1) {
		return nArgs;
	}

	String sEntryFilePath  = args[1];
    String sFilePathTemplate = sEntryFilePath.substr (Zero, sEntryFilePath.find_last_of ('.'));

    String sBinaryFilePath = sFilePathTemplate + BinaryExt;
    String sMetaFilePath = sFilePathTemplate + MetaHeaderExt;

	Log ("Output binary: " << sBinaryFilePath);

	String includeDir;
	if (nArgs > 2) {
		includeDir = args[2];
	}

	auto lexer = new Lexer::Tokenizer ();

	auto compiler = new Compiler ();
	compiler->GenerateInstructionList (
		lexer->ProcessIncludeDirectories (
			lexer->Tokenize (
				ReadFileContent (sEntryFilePath)
			), 
			includeDir
		)
	);
	compiler->LogInstr ();
	VM::Asm assembly = compiler->GetAssemblyCode ();
	VM::ReadOnlyData roData = compiler->GetReadOnlyData ();

    WriteFileContent (sMetaFilePath, compiler->MakeMetaHeader ());

	delete lexer;
	delete compiler;

	Binary::BinaryBuilder::Build (roData, assembly, sBinaryFilePath);

	delete assembly.m_Memory;
	VM::DeleteROData (roData);
	return Zero;
}