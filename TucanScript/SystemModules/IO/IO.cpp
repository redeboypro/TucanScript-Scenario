#include "IO.h"

using namespace TucanScript;

ExternC {
	TucanAPI Undef IO_FileOpen (ExC_Args) {
        stack->Push<Undef*, VM::NATIVEPTR_T> (std::fopen (ExC_StringArg(0), ExC_StringArg(1)), &VM::Word::m_NativePtr);
	}

    TucanAPI Undef IO_FileClose (ExC_Args) {
        stack->Push (std::fclose ((FILE*) ExC_NativePtrArg(0)));
    }

    TucanAPI Undef IO_FilePutC (ExC_Args) {
#if CHAR_MIN < Zero
        stack->Push (std::fputc (ExC_SByteArg(0), (FILE*) ExC_NativePtrArg(1)));
#else
        stack->Push (std::fputc (ExC_ByteArg(0), (FILE*) ExC_NativePtrArg(1)));
#endif
    }

    TucanAPI Undef IO_FileWriteBytes (ExC_Args) {
        stack->Push (std::fwrite (ExC_NativePtrArg(0), ExC_QWordArg(1), ExC_QWordArg(2), (FILE*) ExC_NativePtrArg(3)));
    }

    TucanAPI Undef IO_FileGetC (ExC_Args) {
        SInt32 result = std::fgetc ((FILE*) ExC_NativePtrArg(0));
#if CHAR_MIN < Zero
        stack->Push (result);
#else
        stack->Push (static_cast<UInt8>(result));
#endif
    }

    TucanAPI Undef IO_FileReadBytes (ExC_Args) {
        Undef* buffer = ExC_NativePtrArg(0);
        UInt64 size = ExC_QWordArg(1);
        UInt64 count = ExC_QWordArg(2);
        FILE* file = (FILE*) ExC_NativePtrArg(3);

        stack->Push(std::fread(buffer, size, count, file));
    }

    TucanAPI Undef IO_FileSeek (ExC_Args) {
        FILE* file = (FILE*) ExC_NativePtrArg (0);
        SInt64 offset = ExC_Int64Arg (1);
        SInt32 origin = ExC_Int32Arg (2);
        stack->Push (std::fseek (file, offset, origin));
    }

    TucanAPI Undef IO_FileTell (ExC_Args) {
        stack->Push ((UInt64) std::ftell ((FILE*) ExC_NativePtrArg(0)));
    }

    TucanAPI Undef IO_FileEOF (ExC_Args) {
        stack->Push (std::feof ((FILE*) ExC_NativePtrArg(0)));
    }

    TucanAPI Undef IO_FileFlush (ExC_Args) {
        stack->Push (std::fflush ((FILE*) ExC_NativePtrArg(0)));
    }

    TucanAPI Undef IO_Stdin (ExC_Args) {
        stack->Push<Undef*, VM::NATIVEPTR_T> (stdin, &VM::Word::m_NativePtr);
    }

    TucanAPI Undef IO_Stdout (ExC_Args) {
        stack->Push<Undef*, VM::NATIVEPTR_T> (stdout, &VM::Word::m_NativePtr);
    }

    TucanAPI Undef IO_Stderr (ExC_Args) {
        stack->Push<Undef*, VM::NATIVEPTR_T> (stderr, &VM::Word::m_NativePtr);
    }

    TucanAPI Undef IO_CanReadFD (ExC_Args) {
        SInt32 fd = ExC_Int32Arg(0);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv = {Zero, Zero};
        stack->Push(select(fd + 1, &fds, nullptr, nullptr, &tv) > 0);
    }
}