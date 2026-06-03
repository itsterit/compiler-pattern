#include "compiler.hpp"

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt, const InstructionDef *inst_table, size_t table_size);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// PUBLIC FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool frontend_pass(char **file, uint32_t *size)
{
    if (file != NULL && *file != NULL && size != NULL)
    {
        char *src = *file, *dst = *file, *lineStart = src;
        uint32_t char_cnt = 0, totalSize = *size;

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
                                codeStart = p; // Запоминаем первый не-пробельный символ
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
        *size = (uint32_t)(dst - *file);
        return *size > 0; // Возвращаем true, если файл не пустой
    }
    return false;
}

bool analysis_pass(char **file, uint32_t *size, ParsedFile_t **instructions, uint32_t *instructions_amount)
{
    if ((file != NULL && *file != NULL) && (size != NULL && *size))
    {
        char *buffer = *file;
        uint32_t line_cnt = 0;
        for (uint32_t char_cnt = 0; char_cnt < *size; char_cnt++)
            if (buffer[char_cnt] == '\n')
                line_cnt++;

        ParsedFile_t *parsed_file = (ParsedFile_t *)malloc(sizeof(ParsedFile_t) * line_cnt);
        if (line_cnt && parsed_file != NULL)
        {
            // смотрим содержание файла, отделяем адресацию(origin и метки) от инструкций
            int current_line = 0;
            char *line = strtok(buffer, "\r\n");
            while (line != NULL && current_line <= line_cnt)
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

bool backend_pass(ParsedFile_t *instructions, uint32_t instructions_amount)
{
    if (instructions && instructions_amount)
    {
        for (int cnt = 0; instructions_amount >= cnt; cnt++)
        {
            printf("%s\n\r", instructions[cnt].code_line);
        }
        return 1;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// LOCAL FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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