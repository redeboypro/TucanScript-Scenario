#include "Math.h"

ExternC {
	TucanAPI Undef Sin (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
		if (args->m_Size != 1) {
			LogErr ("Sin function requires 1 argument");
			return;
		}
        stack->Push (std::sin (args->m_Memory[0].m_Data.m_F64));
	}

    TucanAPI Undef Cos (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Cos function requires 1 argument");
            return;
        }
        stack->Push (std::cos (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Tan (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Tan function requires 1 argument");
            return;
        }
        stack->Push (std::tan (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Asin (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Asin function requires 1 argument");
            return;
        }
        stack->Push (std::asin (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Acos (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Acos function requires 1 argument");
            return;
        }
        stack->Push (std::acos (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Atan (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Atan function requires 1 argument");
            return;
        }
        stack->Push (std::atan (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Atan2 (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 2) {
            LogErr ("Atan2 function requires 2 arguments");
            return;
        }
        auto y = args->m_Memory[0].m_Data.m_F64;
        auto x = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::atan2 (y, x));
    }

    TucanAPI Undef Sinh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Sinh function requires 1 argument");
            return;
        }
        stack->Push (std::sinh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Cosh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Cosh function requires 1 argument");
            return;
        }
        stack->Push (std::cosh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Tanh (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Tanh function requires 1 argument");
            return;
        }
        stack->Push (std::tanh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Exp (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Exp function requires 1 argument");
            return;
        }
        stack->Push (std::exp (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Log (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Log function requires 1 argument");
            return;
        }
        stack->Push (std::log (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Sqrt (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Sqrt function requires 1 argument");
            return;
        }
        stack->Push (std::sqrt (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Pow (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 2) {
            LogErr ("Pow function requires 2 arguments");
            return;
        }
        auto base = args->m_Memory[0].m_Data.m_F64;
        auto exp = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::pow (base, exp));
    }

    TucanAPI Undef Ceil (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Ceil function requires 1 argument");
            return;
        }
        stack->Push (std::ceil (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Floor (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Floor function requires 1 argument");
            return;
        }
        stack->Push (std::floor (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Round (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Round function requires 1 argument");
            return;
        }
        stack->Push (std::round (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Abs (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 1) {
            LogErr ("Abs function requires 1 argument");
            return;
        }
        stack->Push (std::abs (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Fmod (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 2) {
            LogErr ("Fmod function requires 2 arguments");
            return;
        }
        auto num = args->m_Memory[0].m_Data.m_F64;
        auto den = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::fmod (num, den));
    }

    TucanAPI Undef Remainder (VM::VirtualMachine* vm, VM::VirtualStack* stack, VM::JmpMemory*, const VM::ValMem* args) {
        if (args->m_Size != 2) {
            LogErr ("Remainder function requires 2 arguments");
            return;
        }
        auto num = args->m_Memory[0].m_Data.m_F64;
        auto den = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::remainder (num, den));
    }
}