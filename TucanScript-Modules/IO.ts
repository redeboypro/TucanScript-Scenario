hIOLib = LoadLibrary("libIO.dylib");

fopen = GetProcAddr(hIOLib, "IO_FileOpen");
fclose = GetProcAddr(hIOLib, "IO_FileClose");
fputc = GetProcAddr(hIOLib, "IO_FilePutC");
fwrite = GetProcAddr(hIOLib, "IO_FileWriteBytes");
fgetc = GetProcAddr(hIOLib, "IO_FileGetC");
fread = GetProcAddr(hIOLib, "IO_FileReadBytes");
fseek = GetProcAddr(hIOLib, "IO_FileSeek");
ftell = GetProcAddr(hIOLib, "IO_FileTell");
feof = GetProcAddr(hIOLib, "IO_FileEOF");
fflush = GetProcAddr(hIOLib, "IO_FileFlush");
stdin = GetProcAddr(hIOLib, "IO_Stdin");
stdout = GetProcAddr(hIOLib, "IO_Stdout");
stderr = GetProcAddr(hIOLib, "IO_Stderr");
fdcont = GetProcAddr(hIOLib, "IO_CanReadFD");