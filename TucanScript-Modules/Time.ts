hTimeLib = LoadLibrary("Time.dll");
GetProcessTime = GetProcAddr(hTimeLib, "GetProcessTime");
GetSysTime = GetProcAddr(hTimeLib, "GetSysTime");
GetClocksPerSec = GetProcAddr(hTimeLib, "GetClocksPerSec");

CLOCKS_PER_SEC = GetClocksPerSec();

def IntDelay(seconds) : start;
def DecDelay(seconds) : start;

imp IntDelay(seconds) {
   start = GetSysTime();
   while (GetSysTime() - start < seconds) {
      #Just wait
   }
}

imp DecDelay(seconds) {
   start = GetProcessTime();
   while (double(GetProcessTime() - start) / double(CLOCKS_PER_SEC) < seconds) {
      #Just wait
   }
}
