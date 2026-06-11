#include "compiler.hpp"

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt, const InstructionDef *inst_table, size_t table_size);
void parse_line_operands(const char *operands_str, ParsedAsmArgs_t *out_args, ParsedFile_t *instructions, uint32_t amount);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// lOGOUT FEATURES /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool save_assembly_listing(const char *filename, ParsedFile_t *instructions, uint32_t instructions_amount)
{
    if (filename && instructions && instructions_amount)
    {
        FILE *f_out = fopen(filename, "wb");
        if (f_out)
        {
            for (uint32_t i = 0; i < instructions_amount; i++)
            {
                printf("\n\r %d", instructions[i].origin);
            }
            // fprintf(f_out, "0x%08X\t%s\n", instructions[i].origin, instructions[i].code_line);
            fclose(f_out);
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool frontend_pass(char *file, uint32_t *size)
{
    uint32_t write_idx = 0, read_idx = 0;
    if (file && size && *size)
    {
        uint32_t original_size = *size;
        while (read_idx < original_size)
        {
            uint32_t line_start_write = write_idx, last_non_space_write = write_idx;
            bool has_content = false, inside_comment = false;

            while (read_idx < original_size)
            {
                char c = file[read_idx++];
                if (c == '\n' || c == '\0')
                {
                    if (has_content)
                    {
                        write_idx = last_non_space_write + 1;
                        if (c == '\n')
                            file[write_idx++] = '\n';
                    }
                    else
                    {
                        write_idx = line_start_write;
                    }
                    break;
                }

                if (c == ';')
                    inside_comment = true;
                if (inside_comment)
                    continue;
                if (!has_content && isspace((unsigned char)c))
                    continue;

                file[write_idx] = (char)toupper((unsigned char)c);
                if (!isspace((unsigned char)file[write_idx]))
                {
                    last_non_space_write = write_idx;
                    has_content = true;
                }
                write_idx++;
            }
        }
        file[write_idx] = '\0';
        *size = write_idx;
        return (*size) > 0;
    }
    return false;
}

bool analysis_pass(char *file, uint32_t size, ParsedFile_t **instructions, uint32_t *instructions_amount)
{
    if (file && size)
    {
        uint32_t line_cnt = 1;
        for (uint32_t char_cnt = 0; char_cnt < size; char_cnt++)
            if (file[char_cnt] == '\n' || file[char_cnt] == '\r')
                line_cnt++;

        ParsedFile_t *parsed_file = (ParsedFile_t *)malloc(sizeof(ParsedFile_t) * line_cnt);
        if (line_cnt && parsed_file != NULL)
        {
            int current_line = 0;
            char *line = strtok(file, "\r\n");
            while (line != NULL && current_line < line_cnt)
            {
                strncpy(parsed_file[current_line].code_line, line, sizeof(parsed_file[current_line].code_line) - 1);
                parsed_file[current_line].code_line[sizeof(parsed_file[current_line].code_line) - 1] = '\0';
                line = strtok(NULL, "\r\n");
                current_line++;
            }

            _calculate_instructions_addresses(parsed_file, line_cnt, (InstructionDef *)&instruction_table, instruction_table_size);
            *instructions = parsed_file;
            *instructions_amount = line_cnt;
            return (true);
        }
    }
    return false;
}

void backend_pass(ParsedFile_t *instructions, uint32_t instructions_amount, InstructionDef *inst_table, size_t table_size)
{
    if (instructions && instructions_amount && inst_table && table_size)
    {
        // 1. Определение точного размера бинарного образа
        uint32_t max_address = 0;
        for (uint32_t i = 0; i < instructions_amount; i++)
        {
            char *text = instructions[i].code_line;
            if (strncmp(text, "ORIGIN", 6) == 0 || strchr(text, ':') != NULL)
                continue;

            char mnem[16] = {0};
            uint8_t size_bytes = 4;
            sscanf(text, "%15s", mnem);
            for (size_t j = 0; j < table_size; j++)
            {
                if (strcmp(mnem, inst_table[j].mnemonic) == 0)
                {
                    size_bytes = inst_table[j].instr_size_bytes;
                    break;
                }
            }

            if (instructions[i].origin + size_bytes > max_address)
            {
                max_address = instructions[i].origin + size_bytes;
            }
        }

        // 2. Выделение памяти под образ
        uint8_t *binary_image = (uint8_t *)malloc(max_address);
        if (binary_image)
        {
            memset(binary_image, 0xFF, max_address);
            {
                // 3. Проход генерации машинного кода
                for (uint32_t i = 0; i < instructions_amount; i++)
                {
                    char *text = instructions[i].code_line;
                    if (strncmp(text, "ORIGIN", 6) == 0 || strchr(text, ':') != NULL)
                        continue;

                    char mnem[16] = {0};
                    sscanf(text, "%15s", mnem);
                    const InstructionDef *def = NULL;
                    for (size_t j = 0; j < table_size; j++)
                    {
                        if (strcmp(mnem, inst_table[j].mnemonic) == 0)
                        {
                            def = &inst_table[j];
                            break;
                        }
                    }
                    if (!def)
                    {
                        fprintf(stderr, "[Error]: Unknown mnemonic %s at %d\n", mnem, i + 1);
                        continue;
                    }

                    const char *operands_ptr = text + strlen(mnem);
                    ParsedAsmArgs_t parsed_args;
                    parse_line_operands(operands_ptr, &parsed_args, instructions, instructions_amount);
                    if (parsed_args.count < def->min_operands || parsed_args.count > def->max_operands)
                    {
                        fprintf(stderr, "[Error]: Incorrect number of operants %s at %d\n", mnem, i + 1);
                        continue;
                    }

                    // ==========================================
                    // ВСТАВКА: Расчет PC-relative смещения для меток
                    // ==========================================
                    for (uint8_t arg_idx = 0; arg_idx < parsed_args.count; arg_idx++)
                    {
                        if (parsed_args.args[arg_idx].type == OPERAND_LABEL)
                        {
                            // В архитектуре ARM Cortex-M3 (Thumb-2) значение программного счетчика (PC)
                            // всегда опережает текущую инструкцию ровно на 4 байта
                            uint32_t current_pc = instructions[i].origin + 4;

                            // В поле .value лежит абсолютный адрес назначения (origin метки)
                            uint32_t target_address = parsed_args.args[arg_idx].value;

                            // Записываем относительное смещение в байтах в поле .offset
                            parsed_args.args[arg_idx].offset = (int32_t)(target_address - current_pc);
                        }
                    }

                    uint32_t machine_code = def->base_opcode;
                    if (def->encode_func != NULL)
                    {
                        machine_code = def->encode_func(def->base_opcode, &parsed_args);
                    }

                    uint32_t write_pos = instructions[i].origin;
                    if (def->instr_size_bytes == 2)
                    {
                        binary_image[write_pos + 0] = (machine_code >> 0x0000) & 0xFF;
                        binary_image[write_pos + 1] = (machine_code >> 0x0008) & 0xFF;
                    }
                    else if (def->instr_size_bytes == 4)
                    {
                        binary_image[write_pos + 0] = (machine_code >> 0x0000) & 0xFF;
                        binary_image[write_pos + 1] = (machine_code >> 0x0008) & 0xFF;
                        binary_image[write_pos + 2] = (machine_code >> 0x0010) & 0xFF;
                        binary_image[write_pos + 3] = (machine_code >> 0x0018) & 0xFF;
                    }
                }
            }
            // Сохранение бинарного образа
            FILE *f_out = fopen("firmware.bin", "wb");
            if (f_out)
            {
                fwrite(binary_image, 1, max_address, f_out);
                fclose(f_out);
                printf("compile complete: %u bytes.\n", max_address);
            }
            free(binary_image);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// LOCAL FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt, const InstructionDef *inst_table, size_t table_size)
{
    uint32_t current_address = 0;
    for (uint32_t i = 0; i < line_cnt; i++)
    {
        uint32_t origin_val = 0;
        char *text = parsed_file[i].code_line;
        parsed_file[i].origin = current_address;

        // if (sscanf(text, "ORIGIN %i", &origin_val) == 1)
        // {
        //     current_address = origin_val;
        //     parsed_file[i].origin = current_address;
        //     continue;
        // }
        // if (strchr(text, ':') != NULL)
        // {
        //     continue;
        // }
        // uint8_t actual_size = 0;
        // for (size_t j = 0; j < table_size; j++)
        // {
        //     size_t mnem_len = strlen(inst_table[j].mnemonic);
        //     if (strncmp(text, inst_table[j].mnemonic, mnem_len) == 0 && (text[mnem_len] == ' ' || text[mnem_len] == '\t' || text[mnem_len] == '\0' || text[mnem_len] == '\r' || text[mnem_len] == '\n'))
        //     {
        //         actual_size = inst_table[j].instr_size_bytes;
        //         break;
        //     }
        // }
        // if (actual_size == 2)
        // {
        //     current_address += 2;
        // }
        // else if (actual_size == 4)
        // {
        //     current_address += 4;
        // }
        // else
        // {
        //     printf("[warning]: undefined instruction at line %d\n", i + 1);
        //     current_address += 4;
        // }
    }
}

uint32_t find_label_address(ParsedFile_t *instructions, uint32_t amount, const char *label_name)
{
    for (uint32_t i = 0; i < amount; i++)
    {
        char *line = instructions[i].code_line;
        if (strchr(line, ':') != NULL)
        {
            char temp_label[50] = {0};
            sscanf(line, "%49[^:]", temp_label);
            if (strcmp(temp_label, label_name) == 0)
                return instructions[i].origin;
        }
    }
    return 0xFFFFFFFF;
}

void parse_line_operands(const char *operands_str, ParsedAsmArgs_t *out_args, ParsedFile_t *instructions, uint32_t amount)
{
    out_args->count = 0;
    if (operands_str && *operands_str != '\0')
    {
        char buf[128];
        strncpy(buf, operands_str, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char *token = strtok(buf, ", \t\r\n");
        while (token != NULL && out_args->count < 4)
        {
            AsmArg_t *arg = &out_args->args[out_args->count];

            // 1. РЕГИСТР (например, R0, R15) или АДРЕС В РЕГИСТРЕ [R1]
            if (token[0] == 'R' || (token[0] == '[' && token[1] == 'R'))
            {
                char *num_ptr = (token[0] == '[') ? &token[2] : &token[1];
                arg->value = atoi(num_ptr);

                if (strchr(token, ']'))
                    arg->type = OPERAND_MEM_REG;
                else if (token[0] == '[')
                    arg->type = OPERAND_MEM_OFFSET;
                else
                    arg->type = OPERAND_REG;
            }
            // 2. СМЕЩЕНИЕ ИЛИ КОНСТАНТА (например, #4], #256)
            else if (token[0] == '#')
            {
                int32_t val = (int32_t)strtol(&token[1], NULL, 0);
                if (out_args->count > 0 && out_args->args[out_args->count - 1].type == OPERAND_MEM_OFFSET)
                {
                    out_args->args[out_args->count - 1].offset = val;
                    token = strtok(NULL, ", \t\r\n");
                    continue;
                }
                else
                {
                    arg->type = OPERAND_IMM;
                    arg->value = (uint32_t)val;
                }
            }
            // 3. МЕТКА (буквенное обозначение, не регистр)
            else if (isalpha((unsigned char)token[0]))
            {
                arg->type = OPERAND_LABEL;
                arg->value = find_label_address(instructions, amount, token);
            }

            out_args->count++;
            token = strtok(NULL, ", \t\r\n");
        }
    }
}