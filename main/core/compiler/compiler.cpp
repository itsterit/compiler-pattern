#include "compiler.hpp"
#include "instructions_sets/system_instructions_sets.hpp"

bool frontend_passes(char **file, uint32_t *size)
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

                // Анализируем строку до комментария или конца строки
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

                // Если в строке есть полезный код, копируем его, переводя в UPPERCASE
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

        // Завершаем строку и обновляем итоговый размер
        *dst = '\0';
        *size = (uint32_t)(dst - *file);
        return *size;
    }
    return 0;
}