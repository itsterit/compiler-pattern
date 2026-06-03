#include "main.hpp"

compilation_middleware_type compile_mw;
int main(int argc, char *argv[])
{
    if ((argc == 2) && (argv))
    {
        // Чтение файла
        compile_mw.firm_image.file = argv[1];
        if (open_file(&compile_mw.firm_image.file, &compile_mw.firm_image.size) == false)
            goto execution_error;
        printf("source file: %s(%d)\n\r", argv[1], compile_mw.firm_image.size);

        // Убираем все лишнее
        if (frontend_pass(&compile_mw.firm_image.file, &compile_mw.firm_image.size) == false)
            goto execution_error;
        printf("frontend passes: %s(%d)\n\r", argv[1], compile_mw.firm_image.size);

        // Вывод упрощенного файла
        if (recreate_file((char *)"frontend_pass.log", compile_mw.firm_image.file, &compile_mw.firm_image.size) == false)
            goto execution_error;

        // Определение адресации
        if (analysis_pass(&compile_mw.firm_image.file, &compile_mw.firm_image.size) == false)
            goto execution_error;
        printf("analysis_pass: %s(%d)\n\r", argv[1], compile_mw.firm_image.size);
    }
    goto end_of_program;

execution_error:
    printf("an error has occurred...");
end_of_program:
    getchar();
    return 0;
}
