#ifndef MATH_H
#define MATH_H

#include "../../VirtualMachine.h"
#undef Log

using namespace TucanScript;

ExternC {
    TucanAPI Undef Sin (ExC_Args);
    TucanAPI Undef Cos (ExC_Args);
    TucanAPI Undef Tan (ExC_Args);
    TucanAPI Undef Asin (ExC_Args);
    TucanAPI Undef Acos (ExC_Args);
    TucanAPI Undef Atan (ExC_Args);
    TucanAPI Undef Atan2 (ExC_Args);
    TucanAPI Undef Sinh (ExC_Args);
    TucanAPI Undef Cosh (ExC_Args);
    TucanAPI Undef Tanh (ExC_Args);
    TucanAPI Undef Exp (ExC_Args);
    TucanAPI Undef Log (ExC_Args);
    TucanAPI Undef Sqrt (ExC_Args);
    TucanAPI Undef Pow (ExC_Args);
    TucanAPI Undef Ceil (ExC_Args);
    TucanAPI Undef Floor (ExC_Args);
    TucanAPI Undef Round (ExC_Args);
    TucanAPI Undef Abs (ExC_Args);
    TucanAPI Undef Fmod (ExC_Args);
    TucanAPI Undef Remainder (ExC_Args);
}

#endif