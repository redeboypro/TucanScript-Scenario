#ifndef MATH_H
#define MATH_H

#include "../../VirtualMachine.h"
#undef Log

using namespace TucanScript;

ExternC {
    TucanAPI Undef Sin (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Cos (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Tan (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Asin (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Acos (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Atan (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Atan2 (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Sinh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Cosh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Tanh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Exp (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Log (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Sqrt (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Pow (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Ceil (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Floor (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Round (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Abs (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Fmod (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
    TucanAPI Undef Remainder (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem*);
}

#endif