#include "compiler.hpp"

bool frontend_passes(char **file, uint32_t *size)
{
    char *src = *file; // Указатель для чтения
    char *dst = *file; // Указатель для записи (сдвигаем код влево)
    uint32_t totalSize = *size;

    char *lineStart = src;
    uint32_t i = 0;

    while (i <= totalSize)
    {
        // Ищем конец текущей строки (символ '\n' или конец файла '\0')
        if (src[i] == '\n' || src[i] == '\0')
        {
            char nextChar = src[i];

            // 1. Анализируем строку от lineStart до src[i]
            char *codeEnd = NULL; // Конец полезного кода в строке
            int hasCode = 0;      // Флаг: есть ли в строке значащие символы (не пробелы и не комментарии)
            int insideComment = 0;

            for (char *p = lineStart; p < &src[i]; p++)
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
                        codeEnd = p + 1; // Запоминаем последний значащий символ + 1
                    }
                }
            }

            // 2. Если в строке есть полезный код, копируем его в dst
            if (hasCode && codeEnd != NULL)
            {
                // Копируем только код до комментария (без лишних пробелов в конце)
                for (char *p = lineStart; p < codeEnd; p++)
                {
                    *dst++ = *p;
                }
                // Если исходная строка заканчивалась переносом, сохраняем его
                if (nextChar == '\n')
                {
                    *dst++ = '\n';
                }
            }

            // Переходим к следующей строке
            if (nextChar == '\0')
            {
                break;
            }
            lineStart = &src[i + 1];
        }
        i++;
    }

    // Добавляем финальный терминальный ноль
    *dst = '\0';

    // Обновляем размер файла (сколько байт реально осталось)
    *size = (uint32_t)(dst - *file);

    // Выводим очищенный файл на экран
    int lineNumber = 1;
    if (*size > 0)
    {
        printf("[%03d] ", lineNumber++);
        for (uint32_t j = 0; j < *size; j++)
        {
            putchar((*file)[j]);
            if ((*file)[j] == '\n' && j + 1 < *size)
            {
                printf("[%03d] ", lineNumber++);
            }
        }
        // Добавляем перенос строки в конце вывода, если его там нет
        if ((*file)[*size - 1] != '\n')
        {
            printf("\n");
        }
    }
    else
    {
        printf("[Файл пуст после очистки]\n");
    }

    return 1;
}