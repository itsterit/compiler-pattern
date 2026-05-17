#ifndef __INSTRUCTIONS_SETS_HPP__
#define __INSTRUCTIONS_SETS_HPP__
#include <stdint.h>

// 1. Перечисление типов операндов
typedef enum
{
    OPERAND_NONE,       // Операнд отсутствует
    OPERAND_REG,        // Регистр общего назначения (например, R0-R15)
    OPERAND_IMM,        // Непосредственное значение / константа (Immediate)
    OPERAND_MEM_REG,    // Адрес в регистре (например, [R1])
    OPERAND_MEM_OFFSET, // Адрес со смещением (например, [R1, #4])
    OPERAND_LABEL       // Метка для переходов (например, loop)
} OperandType;

// 2. Структура для описания ограничений конкретного операнда
typedef struct
{
    OperandType type;  // Тип операнда
    uint8_t size_bits; // Размер операнда в битах (например, для IMM: 5, 8, 12 бит)
    uint8_t flags;     // Флаги: например, только чтение, только запись, signed/unsigned
} OperandDesc;

// 3. Главная структура дескриптора инструкции
typedef struct
{
    const char mnemonic[10];  // Текстовое имя (например, "ADD", "MOV", "B")
    uint32_t base_opcode;     // Базовый бинарный код (маска операции)
    uint32_t opcode_mask;     // Маска для проверки валидности кода
    uint8_t instr_size_bytes; // Размер инструкции в байтах (для Cortex-M3: 2 или 4)

    uint8_t min_operands; // Минимальное количество операндов
    uint8_t max_operands; // Максимальное количество операндов

    // Массив дескрипторов для каждого операнда (максимум 4 для большинства архитектур)
    OperandDesc operands[4];

    // Указатель на функцию-обработчик (энкодер) для финальной сборки байт
    uint32_t (*encode_func)(uint32_t base_op, void *parsed_asm_args);
} InstructionDef;

#endif /* __INSTRUCTIONS_SETS__ _HPP*/