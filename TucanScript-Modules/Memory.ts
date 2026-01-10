hMemoryLib    = LoadLibrary("libMemory.dylib");
ByteAlloc     = GetProcAddr(hMemoryLib, "R_Alloc");
MemCpy        = GetProcAddr(hMemoryLib, "R_MemCpy");
Store         = GetProcAddr(hMemoryLib, "R_Store");
MemoryAt      = GetProcAddr(hMemoryLib, "R_MemOffset");

Align        = GetProcAddr(hMemoryLib, "R_Align");
AllocAligned = GetProcAddr(hMemoryLib, "R_AllocAligned");
StoreAligned = GetProcAddr(hMemoryLib, "R_CpyAligned");
I16Cpy       = GetProcAddr(hMemoryLib, "R_I16Cpy");
I32Cpy       = GetProcAddr(hMemoryLib, "R_I32Cpy");
I64Cpy       = GetProcAddr(hMemoryLib, "R_I64Cpy");
U16Cpy       = GetProcAddr(hMemoryLib, "R_U16Cpy");
U32Cpy       = GetProcAddr(hMemoryLib, "R_U32Cpy");
U64Cpy       = GetProcAddr(hMemoryLib, "R_U64Cpy");
FCpy         = GetProcAddr(hMemoryLib, "R_FCpy");
DCpy         = GetProcAddr(hMemoryLib, "R_DCpy");

Join          = GetProcAddr(hMemoryLib, "Merge");
DirectMemory  = GetProcAddr(hMemoryLib, "DirectMemory");

_UIntPtr     = GetProcAddr(hMemoryLib, "R_PtrToQWord");
_CPtr        = GetProcAddr(hMemoryLib, "R_QWordToPtr");
_StrCat      = GetProcAddr(hMemoryLib, "R_StrCat");
_StrCpy      = GetProcAddr(hMemoryLib, "R_StrCpy");