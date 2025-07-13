hCoroutinesLib = LoadLibrary("Coroutines.dll");

SetTaskAllocProps = GetProcAddr(hCoroutinesLib, "SetCoroutineProps");
WaitForEachTask = GetProcAddr(hCoroutinesLib, "WaitForEachTask");
ResumeTask = GetProcAddr(hCoroutinesLib, "ResumeTask");
CloseTask = GetProcAddr(hCoroutinesLib, "CloseTask");