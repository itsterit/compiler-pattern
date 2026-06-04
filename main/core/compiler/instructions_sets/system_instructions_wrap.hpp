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
    if (parsed_asm_args == NULL)
    {
        return base_op;
    }

    ParsedAsmArgs_t *asm_args = (ParsedAsmArgs_t *)parsed_asm_args;

    // Проверяем, что у нас ровно 2 операнда (например: MOV R1, R2 или MOV R1, #10)
    if (asm_args->count != 2)
    {
        return base_op; // Возвращаем базовый опкод при ошибке аргументов
    }

    AsmArg_t *rd_arg = &asm_args->args[0];  // Первый операнд — регистр назначения (Rd)
    AsmArg_t *src_arg = &asm_args->args[1]; // Второй операнд — источник (Rm или IMM)

    // Защита: первый операнд обязательно должен быть регистром
    if (rd_arg->type != OPERAND_REG)
    {
        return base_op;
    }

    uint32_t rd = rd_arg->value & 0xF; // Ограничиваем индекс регистра 4 битами (R0-R15)
    uint32_t result_opcode = base_op;

    // ВАРИАНТ 1: Копирование из регистра в регистр (MOV Rd, Rm)
    // В Thumb-2 (кодировка T1) базовая маска обычно: 0x4600
    // Биты: Rm находится в битах [6:3], Rd находится в бите [7] (как старший) и [2:0] (как младшие)
    // Для простоты Cortex-M3 поддерживает "High registers" (R0-R15):
    // Маска: 0100 0110 D Rm[3:0] Rd[2:0] -> где D — это 4-й бит регистра Rd
    if (src_arg->type == OPERAND_REG)
    {
        uint32_t rm = src_arg->value & 0xF;

        uint32_t d_bit = (rd >> 3) & 1; // Старший бит регистра Rd (бит 3)
        uint32_t rd_low = rd & 7;       // Младшие 3 бита регистра Rd

        result_opcode = 0x4600; // Стандартный базовый опкод для MOV Rm, Rd
        result_opcode |= (rm << 3);
        result_opcode |= (d_bit << 7);
        result_opcode |= rd_low;
    }

    // ВАРИАНТ 2: Загрузка 8-битной константы (MOV Rd, #imm8)
    // В Thumb-2 (кодировка T1) базовая маска: 0x2000
    // Структура бит: 0010 0[Rd:3] [imm8:8]
    // Данная команда работает только со значениями imm8 от 0 до 255 и регистрами R0-R7
    else if (src_arg->type == OPERAND_IMM)
    {
        uint32_t imm8 = src_arg->value & 0xFF;

        // Cortex-M3 кодировка T1 для MOV с константой требует регистр Low (R0-R7)
        if (rd <= 7)
        {
            result_opcode = 0x2000; // Базовый опкод для MOV Rd, #imm8
            result_opcode |= (rd << 8);
            result_opcode |= imm8;
        }
        else
        {
            // Если Rd > 7 (например R8-R12), Cortex-M3 использует 32-битную кодировку Thumb-2 (MOV.W)
            // Базовый опкод для 32-битного MOV.W Rd, #imm12 обычно равен 0xF2400000
            // Здесь приведена базовая 16-битная архитектура, при необходимости можно расширить до 32-бит
            result_opcode = base_op;
        }
    }
    return result_opcode;
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
    if (parsed_asm_args == NULL)
    {
        return base_op;
    }

    ParsedAsmArgs_t *asm_args = (ParsedAsmArgs_t *)parsed_asm_args;

    // Защита: для LDR всегда нужно минимум 2 операнда (например, LDR R1, [R2, #4] или LDR R1, LABEL)
    if (asm_args->count < 2)
    {
        return base_op;
    }

    AsmArg_t *rt_arg = &asm_args->args[0];  // Первый операнд — регистр назначения (Rt)
    AsmArg_t *mem_arg = &asm_args->args[1]; // Второй операнд — базовый регистр или метка в памяти

    // Первый операнд для LDR обязан быть регистром общего назначения
    if (rt_arg->type != OPERAND_REG)
    {
        return base_op;
    }

    uint32_t rt = rt_arg->value & 0x7; // 16-битные инструкции Thumb работают с регистрами R0-R7
    uint32_t result_opcode = base_op;

    // ВАРИАНТ 1: Загрузка по адресу метки (LDR Rt, LABEL) -> транслируется в PC-relative
    if (mem_arg->type == OPERAND_LABEL)
    {
        uint32_t label_addr = mem_arg->value;

        // В архитектуре ARM Cortex значение PC опережает текущую инструкцию на 4 байта
        // Текущий адрес берется из структуры вашей строки (доступен на этапе backend_pass)
        // Предположим, вы передаете или рассчитываете PC-смещение заранее:
        // В Thumb-2 кодировка T1: 0100 1 [Rt:3] [imm8:8] (смещение должно быть кратно 4)
        result_opcode = 0x4800; // Базовый опкод LDR Rt, [PC, #imm8]

        // Для демонстрации: если вы завели вычисление смещения в mem_arg->offset
        uint32_t imm8 = (mem_arg->offset >= 0) ? (mem_arg->offset >> 2) : 0;

        result_opcode |= (rt << 8);
        result_opcode |= (imm8 & 0xFF);
    }

    // ВАРИАНТ 2: Смещение по регистру и константе (LDR Rt, [Rn, #imm5])
    // Синтаксис: OPERAND_MEM_OFFSET
    // Кодировка T1: 0110 1 [imm5:5] [Rn:3] [Rt:3]
    // В Thumb-2 для 32-битного слова imm5 умножается на 4 при обращении (в опкоде пишется imm5 >> 2)
    else if (mem_arg->type == OPERAND_MEM_OFFSET)
    {
        uint32_t rn = mem_arg->value & 0x7;
        uint32_t imm5 = (mem_arg->offset >> 2) & 0x1F; // Сдвигаем на 2, так как шаг равен 4 байтам

        result_opcode = 0x6800; // Базовый опкод LDR Rt, [Rn, #imm5]
        result_opcode |= (imm5 << 6);
        result_opcode |= (rn << 3);
        result_opcode |= rt;
    }

    // ВАРИАНТ 3: Смещение по двум регистрам (LDR Rt, [Rn, Rm])
    // Синтаксис: OPERAND_MEM_REG. Если передан третий аргумент, он становится Rm
    // Кодировка T1: 0101 100 [Rm:3] [Rn:3] [Rt:3]
    else if (mem_arg->type == OPERAND_MEM_REG || asm_args->count == 3)
    {
        uint32_t rn = mem_arg->value & 0x7;
        uint32_t rm = 0;

        if (asm_args->count == 3)
        {
            rm = asm_args->args[2].value & 0x7;
        }

        result_opcode = 0x5800; // Базовый опкод LDR Rt, [Rn, Rm]
        result_opcode |= (rm << 6);
        result_opcode |= (rn << 3);
        result_opcode |= rt;
    }
    return result_opcode;
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
