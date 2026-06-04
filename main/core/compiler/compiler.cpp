#include "compiler.hpp"

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt, const InstructionDef *inst_table, size_t table_size);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool frontend_pass(char *file, uint32_t size)
{
    if (file && size)
    {
        char *src = file, *dst = file, *lineStart = src;
        uint32_t char_cnt = 0, totalSize = size;

        while (char_cnt <= totalSize)
        {
            if (src[char_cnt] == '\n' || src[char_cnt] == '\0')
            {
                char nextChar = src[char_cnt];
                char *codeStart = NULL;
                char *codeEnd = NULL;
                int insideComment = 0;

                // Ищем начало кода (без пробелов) и его конец (до комментария)
                for (char *p = lineStart; p < &src[char_cnt]; p++)
                {
                    if (*p == ';')
                    {
                        insideComment = 1;
                    }
                    if (!insideComment)
                    {
                        if (!isspace((unsigned char)*p))
                        {
                            if (codeStart == NULL)
                            {
                                codeStart = p;
                            }
                            codeEnd = p + 1;
                        }
                    }
                }

                // Если в строке есть полезный код, копируем его
                if (codeStart != NULL && codeEnd != NULL)
                {
                    for (char *p = codeStart; p < codeEnd; p++)
                    {
                        *dst++ = (char)toupper((unsigned char)*p);
                    }
                    if (nextChar == '\n')
                    {
                        *dst++ = '\n';
                    }
                }

                if (nextChar == '\0')
                {
                    break;
                }
                lineStart = &src[char_cnt + 1];
            }
            char_cnt++;
        }

        *dst = '\0';
        size = (uint32_t)(dst - file);
        return size > 0;
    }
    return false;
}

bool analysis_pass(char *file, uint32_t size, ParsedFile_t **instructions, uint32_t *instructions_amount)
{
    if (file && size)
    {
        uint32_t line_cnt = 0;
        for (uint32_t char_cnt = 0; char_cnt < size; char_cnt++)
            if (file[char_cnt] == '\n')
                line_cnt++;

        ParsedFile_t *parsed_file = (ParsedFile_t *)malloc(sizeof(ParsedFile_t) * line_cnt);
        if (line_cnt && parsed_file != NULL)
        {
            // смотрим содержание файла, отделяем адресацию(origin и метки) от инструкций
            int current_line = 0;
            char *line = strtok(file, "\r\n");
            while (line != NULL && current_line < line_cnt)
            {
                strncpy(parsed_file[current_line].code_line, line, sizeof(parsed_file[current_line].code_line) - 1);
                parsed_file[current_line].code_line[sizeof(parsed_file[current_line].code_line) - 1] = '\0';
                parsed_file[current_line].origin = 0;
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
    // if (!instructions || instructions_amount == 0 || !inst_table)
    //     return;

    // // 1. Определение точного размера бинарного образа
    // uint32_t max_address = 0;
    // for (uint32_t i = 0; i < instructions_amount; i++)
    // {
    //     char *text = instructions[i].code_line;
    //     if (strncmp(text, "ORIGIN", 6) == 0 || strchr(text, ':') != NULL)
    //     {
    //         continue;
    //     }

    //     char mnem[16] = {0};
    //     uint8_t size_bytes = 0;
    //     sscanf(text, "%15s", mnem);
    //     for (size_t j = 0; j < table_size; j++)
    //     {
    //         if (strcmp(mnem, inst_table[j].mnemonic) == 0)
    //         {
    //             size_bytes = inst_table[j].instr_size_bytes;
    //             break;
    //         }
    //     }

    //     if (size_bytes == 0)
    //         size_bytes = 4;
    //     if (instructions[i].origin + size_bytes > max_address)
    //     {
    //         max_address = instructions[i].origin + size_bytes;
    //     }
    // }

    // // 2. Выделение памяти под образ
    // uint8_t *binary_image = (uint8_t *)malloc(max_address);
    // if (!binary_image)
    //     return;
    // memset(binary_image, 0xFF, max_address);

    // // 3. Проход генерации машинного кода
    // for (uint32_t i = 0; i < instructions_amount; i++)
    // {
    //     char *text = instructions[i].code_line;
    //     if (strncmp(text, "ORIGIN", 6) == 0 || strchr(text, ':') != NULL)
    //     {
    //         continue;
    //     }

    //     char mnem[16] = {0};
    //     sscanf(text, "%15s", mnem);
    //     const InstructionDef *def = NULL;
    //     for (size_t j = 0; j < table_size; j++)
    //     {
    //         if (strcmp(mnem, inst_table[j].mnemonic) == 0)
    //         {
    //             def = &inst_table[j];
    //             break;
    //         }
    //     }
    //     if (!def)
    //     {
    //         fprintf(stderr, "Error: Unknown mnemonic %s at %d\n", mnem, i + 1);
    //         continue;
    //     }

    //     const char *operands_ptr = text + strlen(mnem);
    //     ParsedAsmArgs_t parsed_args;
    //     parse_line_operands(operands_ptr, &parsed_args, instructions, instructions_amount);

    //     if (parsed_args.count < def->min_operands || parsed_args.count > def->max_operands)
    //     {
    //         fprintf(stderr, "Ошибка: Неверное количество операндов для %s на строке %d\n", mnem, i + 1);
    //         continue;
    //     }

    //     // 4. ВЫЗОВ ЭНКОДЕРА
    //     uint32_t machine_code = def->base_opcode;
    //     if (def->encode_func != NULL)
    //     {
    //         machine_code = def->encode_func(def->base_opcode, &parsed_args);
    //     }

    //     // Накладываем маску операции
    //     machine_code = (machine_code & ~def->opcode_mask) | (def->base_opcode & def->opcode_mask);

    //     // 5. ПОБАЙТОВАЯ ЗАПИСЬ С УЧЕТОМ РЕАЛЬНОГО РАЗМЕРА ИНСТРУКЦИИ
    //     uint32_t write_pos = instructions[i].origin;

    //     if (def->instr_size_bytes == 2)
    //     {
    //         // Записываем только 2 байта (16 бит)
    //         binary_image[write_pos + 0] = (machine_code >> 0) & 0xFF;
    //         binary_image[write_pos + 1] = (machine_code >> 8) & 0xFF;
    //     }
    //     else if (def->instr_size_bytes == 4)
    //     {
    //         // Записываем все 4 байта (32 бита)
    //         binary_image[write_pos + 0] = (machine_code >> 0) & 0xFF;
    //         binary_image[write_pos + 1] = (machine_code >> 8) & 0xFF;
    //         binary_image[write_pos + 2] = (machine_code >> 16) & 0xFF;
    //         binary_image[write_pos + 3] = (machine_code >> 24) & 0xFF;
    //     }
    // }

    // // Сохранение бинарного образа
    // FILE *f_out = fopen("firmware.bin", "wb");
    // if (f_out)
    // {
    //     fwrite(binary_image, 1, max_address, f_out);
    //     fclose(f_out);
    //     printf("Образ успешно сохранен. Итоговый размер: %u байт.\n", max_address);
    // }
    // free(binary_image);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// LOCAL FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void save_assembly_listing(const char *filename, ParsedFile_t *instructions, uint32_t instructions_amount)
{
    if (filename && instructions && instructions_amount)
    {
        FILE *f_out = fopen(filename, "wb");
        if (f_out)
        {
            for (uint32_t i = 0; i < instructions_amount; i++)
            {
                // Выводим адрес в 8-значном 16-ричном формате с ведущими нулями (например, 0x00004600)
                // \t делает аккуратный отступ перед текстом инструкции
                fprintf(f_out, "0x%08X\t%s\n", instructions[i].origin, instructions[i].code_line);
            }
            fclose(f_out);
        }
    }
}

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt, const InstructionDef *inst_table, size_t table_size)
{
    uint32_t current_address = 0;

    for (uint32_t i = 0; i <= line_cnt; i++)
    {
        char *text = parsed_file[i].code_line;
        uint32_t origin_val = 0;

        parsed_file[i].origin = current_address;
        if (sscanf(text, "ORIGIN %i", &origin_val) == 1)
        {
            current_address = origin_val;
            parsed_file[i].origin = current_address;
            continue;
        }
        if (strchr(text, ':') != NULL)
        {
            continue;
        }

        uint8_t actual_size = 0;
        for (size_t j = 0; j < table_size; j++)
        {
            size_t mnem_len = strlen(inst_table[j].mnemonic);

            // Сверяем начало строки с мнемоникой из таблицы (строго в UPPER CASE)
            if (strncmp(text, inst_table[j].mnemonic, mnem_len) == 0 &&
                (text[mnem_len] == ' ' || text[mnem_len] == '\t' || text[mnem_len] == '\0' || text[mnem_len] == '\r' || text[mnem_len] == '\n'))
            {
                actual_size = inst_table[j].instr_size_bytes;
                break;
            }
        }

        if (actual_size == 2)
        {
            current_address += 2;
        }
        else if (actual_size == 4)
        {
            current_address += 4;
        }
        else
        {
            fprintf(stderr, "warning: undefined instruction at line %d: %s\n", i + 1, text);
            current_address += 4;
        }
    }
}