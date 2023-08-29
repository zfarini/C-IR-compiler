#include <stdint.h>
#include <stdio.h>
#include <limits.h>

enum RValue_Type
{
	RV_U64 = 1,
    RV_U32,
    RV_U16,
    RV_U8,
    RV_I64,
    RV_I32,
    RV_I16,
    RV_I8,
    RV_PTR,
    RV_F32,
    RV_F64,
    RVALUE_COUNT,
};

typedef struct RValue {
	union {
		uint64_t u64;
		uint32_t u32;
		uint16_t u16;
		uint8_t	 u8;
		int64_t  i64;
		int32_t  i32;
		int16_t  i16;
		int8_t	 i8;
        float f32;
        double f64;
	};
	int type;
} RValue;

RValue add_rvalues(RValue r1, RValue r2)
{
    RValue result;
    result.type = r1.type;

    switch (r1.type)
    {
        case RV_U64:
        case RV_U32:
        case RV_U16:
        case RV_U8:
            result.u64 = r1.u64 / r2.u64;
			break ;
		case RV_I64:
        case RV_I32:
        case RV_I16:
        case RV_I8:
            result.i64 = r1.i64 <= r2.i64;
			break ;
        case RV_PTR:
            // Handle pointer addition
            break;
        case RV_F32:
        case RV_F64:
            result.i32 = r1.f64 < r2.f64; // Use the larger type to store the sum
            break;
    }

    return result;
}

int main()
{
	RValue r1 = {.type = RV_F32, .f32 = 421421.1616};
	RValue r2 = {.type = RV_F32, .f32 = -32.4512};
	RValue res = add_rvalues(r1, r2);
	printf("%d\n", res.i32);
	printf("%d\n", r1.f32 < r2.f32);
}