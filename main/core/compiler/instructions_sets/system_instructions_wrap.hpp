#include "system_instructions_sets.hpp"

typedef struct
{
    OperandType type;
    uint32_t value;
    int32_t offset;
} AsmArg_t;
typedef struct
{
    AsmArg_t args[4];
    uint8_t count;
} ParsedAsmArgs_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// MNEMONICS AND ARGS HANDLER ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// DATA MOVEMENT
static uint32_t encode_func__mov(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

/// ARITHMETIC
static uint32_t encode_func__add(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__sub(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}
static uint32_t encode_func__mul(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

/// LOGICAL & SHIFTS
static uint32_t encode_func__and(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__orr(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__lsl(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__lsr(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

/// LOAD/STORE
static uint32_t encode_func__ldr(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__str(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

/// BRANCHES & COMPARISONS
static uint32_t encode_func__cmp(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__b(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__bl(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__bx(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

/// STACK OPERATIONS
static uint32_t encode_func__push(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}

static uint32_t encode_func__pop(uint32_t base_op, void *parsed_asm_args)
{
    return base_op;
}
