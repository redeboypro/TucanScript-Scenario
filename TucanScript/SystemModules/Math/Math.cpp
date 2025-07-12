#include "Math.h"

ExternC {
	TucanAPI Undef Sin (ExC_Args) {
		if (args->m_Size != 1) {
			LogErr ("Sin function requires 1 argument");
			return;
		}
        stack->Push (std::sin (args->m_Memory[0].m_Data.m_F64));
	}

    TucanAPI Undef Cos (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Cos function requires 1 argument");
            return;
        }
        stack->Push (std::cos (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Tan (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Tan function requires 1 argument");
            return;
        }
        stack->Push (std::tan (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Asin (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Asin function requires 1 argument");
            return;
        }
        stack->Push (std::asin (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Acos (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Acos function requires 1 argument");
            return;
        }
        stack->Push (std::acos (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Atan (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Atan function requires 1 argument");
            return;
        }
        stack->Push (std::atan (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Atan2 (ExC_Args) {
        if (args->m_Size != 2) {
            LogErr ("Atan2 function requires 2 arguments");
            return;
        }
        auto y = args->m_Memory[0].m_Data.m_F64;
        auto x = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::atan2 (y, x));
    }

    TucanAPI Undef Sinh (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Sinh function requires 1 argument");
            return;
        }
        stack->Push (std::sinh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Cosh (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Cosh function requires 1 argument");
            return;
        }
        stack->Push (std::cosh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Tanh (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Tanh function requires 1 argument");
            return;
        }
        stack->Push (std::tanh (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Exp (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Exp function requires 1 argument");
            return;
        }
        stack->Push (std::exp (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Log (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Log function requires 1 argument");
            return;
        }
        stack->Push (std::log (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Sqrt (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Sqrt function requires 1 argument");
            return;
        }
        stack->Push (std::sqrt (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Pow (ExC_Args) {
        if (args->m_Size != 2) {
            LogErr ("Pow function requires 2 arguments");
            return;
        }
        auto base = args->m_Memory[0].m_Data.m_F64;
        auto exp = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::pow (base, exp));
    }

    TucanAPI Undef Ceil (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Ceil function requires 1 argument");
            return;
        }
        stack->Push (std::ceil (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Floor (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Floor function requires 1 argument");
            return;
        }
        stack->Push (std::floor (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Round (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Round function requires 1 argument");
            return;
        }
        stack->Push (std::round (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Abs (ExC_Args) {
        if (args->m_Size != 1) {
            LogErr ("Abs function requires 1 argument");
            return;
        }
        stack->Push (std::abs (args->m_Memory[0].m_Data.m_F64));
    }

    TucanAPI Undef Fmod (ExC_Args) {
        if (args->m_Size != 2) {
            LogErr ("Fmod function requires 2 arguments");
            return;
        }
        auto num = args->m_Memory[0].m_Data.m_F64;
        auto den = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::fmod (num, den));
    }

    TucanAPI Undef Remainder (ExC_Args) {
        if (args->m_Size != 2) {
            LogErr ("Remainder function requires 2 arguments");
            return;
        }
        auto num = args->m_Memory[0].m_Data.m_F64;
        auto den = args->m_Memory[1].m_Data.m_F64;
        stack->Push (std::remainder (num, den));
    }
}