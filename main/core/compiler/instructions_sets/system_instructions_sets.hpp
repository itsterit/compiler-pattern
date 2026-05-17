#ifndef __SYSTEM_INSTRUCTIONS_SETS_HPP_
#define __SYSTEM_INSTRUCTIONS_SETS_HPP_
#include "instructions_definitions.hpp"

static const InstructionDef instruction_table[] =
    {
        // ПЕРЕСЫЛКА ДАННЫХ (DATA MOVEMENT)
        {
            // MOV R1, R2 (16-бит) — Копирование регистра в регистр
            .mnemonic = "MOV",
            .base_opcode = 0x4600,
            .opcode_mask = 0xFF00,
            .instr_size_bytes = 2,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}},
        },
        {
            // MOV R1, #255 (16-бит) — Загрузка 8-битной константы
            .mnemonic = "MOV",
            .base_opcode = 0x2000,
            .opcode_mask = 0xF800,
            .instr_size_bytes = 2,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 3, 0}, {OPERAND_IMM, 8, 0}},
        },
        {
            // MOVW R1, #0xABCD (32-бит) — Загрузка 16-битной константы в нижнюю часть регистра
            .mnemonic = "MOVW",
            .base_opcode = 0xF2400000,
            .opcode_mask = 0xFB500000,
            .instr_size_bytes = 4,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_IMM, 16, 0}},
        },

        // АРИФМЕТИКА (ARITHMETIC)
        {
            // ADD R1, R2, R3 (32-бит) — Сложение двух регистров
            .mnemonic = "ADD",
            .base_opcode = 0xEB100000,
            .opcode_mask = 0xFFF00000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}},
        },
        {
            // SUB R1, R2, #100 (32-бит) — Вычитание константы (до 12 бит)
            .mnemonic = "SUB",
            .base_opcode = 0xF1A00000,
            .opcode_mask = 0xFBFF0000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_IMM, 12, 0}},
        },
        {
            // MUL R1, R2, R3 (32-бит) — Умножение (в Cortex-M3/M4 всегда 32-битное)
            .mnemonic = "MUL",
            .base_opcode = 0xFB00F000,
            .opcode_mask = 0xFFF0F000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}},
        },

        // ЛОГИКА И СДВИГИ (LOGICAL & SHIFTS)
        {
            // AND R1, R2, R3 (32-бит) — Побитовое И
            .mnemonic = "AND",
            .base_opcode = 0xEA000000,
            .opcode_mask = 0xFFF00000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}},
        },
        {
            // ORR R1, R2, R3 (32-бит) — Побитовое ИЛИ
            .mnemonic = "ORR",
            .base_opcode = 0xEA400000,
            .opcode_mask = 0xFFF00000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}},
        },
        {
            // LSL R1, R2, #5 (32-бит) — Логический сдвиг влево на константу (до 5 бит)
            .mnemonic = "LSL",
            .base_opcode = 0xEA4F0000,
            .opcode_mask = 0xFFEF0000,
            .instr_size_bytes = 4,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_REG, 4, 0}, {OPERAND_IMM, 5, 0}},
        },
        {
            // LSR R1, R2, #5 (16-бит) — Логический сдвиг вправо на константу (заполнение нулями)
            .mnemonic = "LSR",
            .base_opcode = 0x0800,
            .opcode_mask = 0xF800,
            .instr_size_bytes = 2,
            .min_operands = 3,
            .max_operands = 3,
            .operands = {{OPERAND_REG, 3, 0}, {OPERAND_REG, 3, 0}, {OPERAND_IMM, 5, 0}},
        },

        // РАБОТА С ПАМЯТЬЮ (LOAD/STORE)
        {
            // LDR R1, [R2, #4] (32-бит) — Загрузка 32-битного слова из памяти со смещением
            .mnemonic = "LDR",
            .base_opcode = 0xF8D00000,
            .opcode_mask = 0xFFF00000,
            .instr_size_bytes = 4,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 4, 0}, {OPERAND_MEM_OFFSET, 12, 0}},
        },
        {
            // STR R1, [R2] (16-бит) — Сохранение слова в память без смещения (базовый регистр)
            .mnemonic = "STR",
            .base_opcode = 0x6000,
            .opcode_mask = 0xF800,
            .instr_size_bytes = 2,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 3, 0}, {OPERAND_MEM_REG, 3, 0}},
        },

        // ПЕРЕХОДЫ И СРАВНЕНИЯ (BRANCHES & COMPARISONS)
        {
            // CMP R1, R2 (16-бит) — Сравнение двух регистров (выставляет флаги)
            .mnemonic = "CMP",
            .base_opcode = 0x4280,
            .opcode_mask = 0xFFC0,
            .instr_size_bytes = 2,
            .min_operands = 2,
            .max_operands = 2,
            .operands = {{OPERAND_REG, 3, 0}, {OPERAND_REG, 3, 0}},
        },
        {
            // B loop (16-бит) — Безусловный относительный переход
            .mnemonic = "B",
            .base_opcode = 0xE000,
            .opcode_mask = 0xF800,
            .instr_size_bytes = 2,
            .min_operands = 1,
            .max_operands = 1,
            .operands = {{OPERAND_LABEL, 11, 0}},
        },
        {
            // BL function (32-бит) — Вызов функции (Переход с сохранением ссылки в LR)
            .mnemonic = "BL",
            .base_opcode = 0xF000F800,
            .opcode_mask = 0xF800D000,
            .instr_size_bytes = 4,
            .min_operands = 1,
            .max_operands = 1,
            .operands = {{OPERAND_LABEL, 24, 0}},
        },
        {
            // BX LR (16-бит) — Переход по адресу в регистре (обычно возврат из функции)
            .mnemonic = "BX",
            .base_opcode = 0x4700,
            .opcode_mask = 0xFF80,
            .instr_size_bytes = 2,
            .min_operands = 1,
            .max_operands = 1,
            .operands = {{OPERAND_REG, 4, 0}},

        },

        // РАБОТА СО СТЕКОМ (STACK OPERATIONS)
        {
            // PUSH {R4-R7, LR} (16-бит) — Сохранение регистров в стек
            .mnemonic = "PUSH",
            .base_opcode = 0xB400,
            .opcode_mask = 0xFF00,
            .instr_size_bytes = 2,
            .min_operands = 1,
            .max_operands = 1,
            .operands = {{OPERAND_REG, 9, 0}},
        },
        {
            // POP {R4-R7, PC} (16-бит) — Восстановление регистров из стека (и возврат, если есть PC)
            .mnemonic = "POP",
            .base_opcode = 0xBC00,
            .opcode_mask = 0xFF00,
            .instr_size_bytes = 2,
            .min_operands = 1,
            .max_operands = 1,
            .operands = {{OPERAND_REG, 9, 0}},
        },
};

#endif /* __SYSTEM_INSTRUCTIONS_SETS_HPP_ */