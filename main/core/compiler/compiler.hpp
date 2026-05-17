#ifndef __COMPILER_HPP__
#define __COMPILER_HPP__
#include "main.hpp"
#include "instructions_sets/instructions_definitions.hpp"
#include "instructions_sets/system_instructions_sets.hpp"

typedef struct
{
    char code_line[50];
    InstructionDef instruction;
    uint32_t instruction_execution_address;
} ParsedFile_t;

bool frontend_pass(char **file, uint32_t *size);
bool analysis_pass(char *file, uint32_t *size);

#endif /* __COMPILER_HPP__ */
