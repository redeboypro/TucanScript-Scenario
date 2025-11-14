#ifndef IO_H
#define IO_H

#include "../../VirtualMachine.h"
#undef _Exit

#include <unistd.h>

using namespace TucanScript;

ExternC {
	TucanAPI Undef IO_FileOpen (ExC_Args);
    TucanAPI Undef IO_FileClose (ExC_Args);
    TucanAPI Undef IO_FilePutC (ExC_Args);
    TucanAPI Undef IO_FilePutW (ExC_Args);
    TucanAPI Undef IO_FileWriteBytes (ExC_Args);
    TucanAPI Undef IO_FileGetC (ExC_Args);
    TucanAPI Undef IO_FileReadBytes (ExC_Args);
    TucanAPI Undef IO_FileSeek (ExC_Args);
    TucanAPI Undef IO_FileTell (ExC_Args);
    TucanAPI Undef IO_FileEOF (ExC_Args);
    TucanAPI Undef IO_FileFlush (ExC_Args);
    TucanAPI Undef IO_Stdin (ExC_Args);
    TucanAPI Undef IO_Stdout (ExC_Args);
    TucanAPI Undef IO_Stderr (ExC_Args);
    TucanAPI Undef IO_CanReadFD (ExC_Args);
}

#endif