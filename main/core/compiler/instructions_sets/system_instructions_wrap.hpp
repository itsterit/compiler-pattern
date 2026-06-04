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
// page286: A6.7.75 MOV (immediate)
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
    if (parsed_asm_args == NULL)
    {
        return base_op;
    }

    ParsedAsmArgs_t *asm_args = (ParsedAsmArgs_t *)parsed_asm_args;

    // Защита: для SUB нужно как минимум 2 операнда
    if (asm_args->count < 2)
    {
        return base_op;
    }

    AsmArg_t *rd_arg = &asm_args->args[0]; // Регистр назначения (Rd)

    if (rd_arg->type != OPERAND_REG)
    {
        return base_op;
    }

    uint32_t rd = rd_arg->value & 0x7; // 16-битные Thumb инструкции работают с Low-регистрами R0-R7
    uint32_t result_opcode = base_op;

    // ВАРИАНТ 1: Вычитание константы (SUB Rd, Rn, #imm3 или SUB Rd, #imm8)
    if (asm_args->args[asm_args->count - 1].type == OPERAND_IMM)
    {
        uint32_t imm = asm_args->args[asm_args->count - 1].value;

        if (asm_args->count == 3)
        {
            // Формат: SUB Rd, Rn, #imm3 (Encoding T1)
            // Битовая маска: 0001 111 [imm3:3] [Rn:3] [Rd:3]
            uint32_t rn = asm_args->args[1].value & 0x7;

            result_opcode = 0x1E00; // Базовый опкод для 3-битного imm
            result_opcode |= ((imm & 0x7) << 6);
            result_opcode |= (rn << 3);
            result_opcode |= rd;
        }
        else if (asm_args->count == 2)
        {
            // Формат: SUB Rd, #imm8 (Encoding T2)
            // Битовая маска: 0011 1 [Rd:3] [imm8:8]
            // (В данном случае вычитание происходит из самого Rd)
            result_opcode = 0x3800; // Базовый опкод для 8-битного imm
            result_opcode |= (rd << 8);
            result_opcode |= (imm & 0xFF);
        }
    }
    // ВАРИАНТ 2: Вычитание регистров (SUB Rd, Rn, Rm)
    else if (asm_args->args[asm_args->count - 1].type == OPERAND_REG)
    {
        if (asm_args->count == 3)
        {
            // Формат: SUB Rd, Rn, Rm (Encoding T1)
            // Битовая маска: 0001 101 [Rm:3] [Rn:3] [Rd:3]
            uint32_t rn = asm_args->args[1].value & 0x7;
            uint32_t rm = asm_args->args[2].value & 0x7;

            result_opcode = 0x1A00; // Базовый опкод для SUB регистров
            result_opcode |= (rm << 6);
            result_opcode |= (rn << 3);
            result_opcode |= rd;
        }
        else if (asm_args->count == 2)
        {
            // Если написано SUB R1, R2 -> превращаем в SUB R1, R1, R2
            uint32_t rm = asm_args->args[1].value & 0x7;

            result_opcode = 0x1A00;
            result_opcode |= (rm << 6);
            result_opcode |= (rd << 3); // Rn равен Rd
            result_opcode |= rd;
        }
    }

    return result_opcode;
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
// page190: A6.7.21 CBNZ, CBZ
static uint32_t encode_func__cbz(uint32_t base_op, void *parsed_asm_args)
{
    if (parsed_asm_args == NULL)
    {
        return base_op;
    }

    ParsedAsmArgs_t *asm_args = (ParsedAsmArgs_t *)parsed_asm_args;

    // Защита: CBZ строго требует 2 операнда (Регистр и Метка)
    if (asm_args->count != 2)
    {
        return base_op;
    }

    AsmArg_t *rn_arg = &asm_args->args[0];    // Проверяемый регистр
    AsmArg_t *label_arg = &asm_args->args[1]; // Целевая метка

    // Валидация типов аргументов
    if (rn_arg->type != OPERAND_REG || label_arg->type != OPERAND_LABEL)
    {
        return base_op;
    }

    uint32_t rn = rn_arg->value & 0x7; // CBZ работает только с регистрами R0-R7
    uint32_t result_opcode = 0xB100;   // Базовый опкод CBZ (1011 0001 0000 0000)

    // Вычисляем смещение.
    // В ARM Cortex значение PC всегда на 4 байта впереди текущей инструкции.
    // label_arg->value хранит абсолютный origin метки.
    // Нам также нужен текущий адрес (передайте его через label_arg->offset из backend_pass)
    // Допустим: label_arg->offset = label_absolute_address - (current_instruction_address + 4);
    int32_t byte_offset = label_arg->offset;

    // Смещение должно быть положительным (вперед) и не превышать 126 байт для CBZ
    if (byte_offset > 0 && byte_offset <= 126)
    {
        // Так как переходы выровнены по полуслову, делим смещение на 2
        uint32_t imm6 = (uint32_t)(byte_offset >> 1);

        uint32_t imm5 = imm6 & 0x1F;      // Младшие 5 бит смещения
        uint32_t i_bit = (imm6 >> 5) & 1; // 6-й (старший) бит смещения

        // Собираем инструкцию по спецификации ARM Thumb
        result_opcode |= rn;           // Биты [2:0] -> регистр Rn
        result_opcode |= (imm5 << 3);  // Биты [7:3] -> imm5
        result_opcode |= (i_bit << 9); // Бит [9]    -> i-bit
    }
    else
    {
        // Если смещение отрицательное или слишком большое, CBZ использовать нельзя.
        // В реальном ассемблере здесь генерируется ошибка или подменяется на CMP + BNE
        fprintf(stderr, "Error: Offset for CBZ is out of range (0-126 bytes)\n");
    }

    return result_opcode;
}

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
