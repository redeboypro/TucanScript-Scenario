hCoroutinesLib = LoadLibrary("libCoroutines.dylib");

SetAsyncOptions = GetProcAddr(hCoroutinesLib, "SetCoroutineProps");
GoForEachAsync  = GetProcAddr(hCoroutinesLib, "WaitForEachTask");
GoAsync         = GetProcAddr(hCoroutinesLib, "ResumeTask");
StopAsync       = GetProcAddr(hCoroutinesLib, "CloseTask");