hMemoryLib = LoadLibrary("libMemory.dylib");

malloc = GetProcAddr(hMemoryLib, "R_Alloc");
PtrToQWORD = GetProcAddr(hMemoryLib, "R_PtrToQWord");
QWORDToPtr = GetProcAddr(hMemoryLib, "R_QWordToPtr");
strcat = GetProcAddr(hMemoryLib, "R_StrCat");
strcpy = GetProcAddr(hMemoryLib, "R_StrCpy");
Join = GetProcAddr(hMemoryLib, "Merge");
GetRawBuf = GetProcAddr(hMemoryLib, "GetRawBuf");