hTimeLib = LoadLibrary("Time.dll");
GetProcessTime = GetProcAddr(hTimeLib, "GetProcessTime");
GetSysTime = GetProcAddr(hTimeLib, "GetSysTime");
GetClocksPerSec = GetProcAddr(hTimeLib, "GetClocksPerSec");

CLOCKS_PER_SEC = GetClocksPerSec();

def WaitForSeconds(seconds) : start;
def WaitForSecondsF(seconds) : start;

imp WaitForSeconds(seconds) {
   start = GetSysTime();
   while (GetSysTime() - start < seconds) {
      Yield();
   }
}

imp WaitForSecondsF(seconds) {
   start = GetProcessTime();
   while (double(GetProcessTime() - start) / double(CLOCKS_PER_SEC) < seconds) {
      Yield();
   }
}
