#include "config.hpp"

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
        if (frontend_pass(compile_mw.firm_image.file, compile_mw.firm_image.size) == false)
            goto execution_error;
        printf("frontend passes: %s(%d)\n\n\r", argv[1], compile_mw.firm_image.size);

        // Определение адресации
        if (analysis_pass(compile_mw.firm_image.file, compile_mw.firm_image.size, &compile_mw.preexecutable.instructions, &compile_mw.preexecutable.instructions_amount) == false)
            goto execution_error;
        printf("analysis_pass: %s(%d)\n\n\r", argv[1], compile_mw.preexecutable.instructions_amount);
        save_assembly_listing((char *)"analysis_pass.lst", compile_mw.preexecutable.instructions, compile_mw.preexecutable.instructions_amount);
        free(compile_mw.firm_image.file);

        backend_pass(compile_mw.preexecutable.instructions, compile_mw.preexecutable.instructions_amount, (InstructionDef *)&instruction_table, instruction_table_size);
        printf("done...\n\r");
    }
    goto end_of_program;

execution_error:
    printf("an error has occurred...");
end_of_program:
    getchar();
    return 0;
}
