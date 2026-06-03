#ifndef __COMPILER_HPP__
#define __COMPILER_HPP__
#include "main.hpp"
#include "instructions_sets/instructions_definitions.hpp"
#include "instructions_sets/system_instructions_sets.hpp"

typedef struct
{
    char code_line[50];
    uint32_t origin;
    InstructionDef instruction;
} ParsedFile_t;

bool frontend_pass(char **file, uint32_t *size);
bool analysis_pass(char **file, uint32_t *size, ParsedFile_t **instructions, uint32_t *instructions_amount);
bool backend_pass(ParsedFile_t *instructions, uint32_t instructions_amount);

#endif /* __COMPILER_HPP__ */
