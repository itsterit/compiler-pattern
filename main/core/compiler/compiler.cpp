#include "compiler.hpp"

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt);

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
            // Ищем конец текущей строки (символ '\n' или конец файла '\0')
            if (src[char_cnt] == '\n' || src[char_cnt] == '\0')
            {
                char nextChar = src[char_cnt], *codeEnd = NULL;
                int hasCode = 0, insideComment = 0;
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
                            hasCode = 1;
                            codeEnd = p + 1;
                        }
                    }
                }

                if (hasCode && codeEnd != NULL)
                {
                    for (char *p = lineStart; p < codeEnd; p++)
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
        return *size;
    }
    return 0;
}

bool analysis_pass(char **file, uint32_t *size)
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

            _calculate_instructions_addresses(parsed_file, line_cnt);

            // _calculate_instructions_addresses(parsed_file, line_cnt);
            for (current_line = 0; current_line <= line_cnt; current_line++)
                printf("origin:=%4d   %s\n\r", parsed_file[current_line].origin, parsed_file[current_line].code_line);

            return (current_line == line_cnt);
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// LOCAL FUNCTIONS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _calculate_instructions_addresses(ParsedFile_t *parsed_file, uint32_t line_cnt)
{
    uint32_t current_address = 0;

    for (uint32_t i = 0; i < line_cnt; i++)
    {
        char *text = parsed_file[i].code_line;
        uint32_t origin_val = 0;

        // Присваиваем текущий адрес строке сразу
        parsed_file[i].origin = current_address;

        // 1. Проверка на ORIGIN (строка начинается строго с "ORIGIN")
        if (sscanf(text, "ORIGIN %i", &origin_val) == 1)
        {
            current_address = origin_val;
            parsed_file[i].origin = current_address; // Обновляем адрес для самой директивы
            continue;
        }

        // 2. Проверка на МЕТКУ (наличие двоеточия в строке)
        if (strchr(text, ':') != NULL)
        {
            continue; // Адрес не увеличивается, переходим к следующей строке
        }

        // 3. ИНСТРУКЦИЯ (гарантировано по вашему условию)
        // Любая валидная инструкция разворачивается в 4 байта
        current_address += 4;
    }
}