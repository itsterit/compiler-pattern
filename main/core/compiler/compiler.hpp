#ifndef __COMPILER_HPP__
#define __COMPILER_HPP__
#include "main.hpp"
#include "instructions_sets/instructions_definitions.hpp"
#include "instructions_sets/system_instructions_sets.hpp"

typedef struct
{
    char code_line[100];
    uint32_t origin;
    InstructionDef instruction;
} ParsedFile_t;
bool save_assembly_listing(const char *filename, ParsedFile_t *instructions, uint32_t instructions_amount);

bool frontend_pass(char *file, uint32_t *size);
bool analysis_pass(char *file, uint32_t size, ParsedFile_t **instructions, uint32_t *instructions_amount);
void backend_pass(ParsedFile_t *instructions, uint32_t instructions_amount, InstructionDef *inst_table, size_t table_size);

#endif /* __COMPILER_HPP__ */
