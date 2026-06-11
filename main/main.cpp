#include "config.hpp"

int main(int argc, char *argv[])
{
    volatile char *selected_file_p = argv[1];
    volatile uint32_t selected_file_size = 0;
    volatile ParsedFile_t *directives_set = 0;
    volatile uint32_t instructions_number = 0;

    while ((argc == 2) && (argv))
    {
        printf("\n\rstarting...\n\r");
        {
            EXECUTION_CHECK(!open_file((char **)&selected_file_p, (uint32_t *)&selected_file_size));
            printf("selected file: %s(%4.d bytes)\n\r", argv[1], selected_file_size);

            EXECUTION_CHECK(!frontend_pass((char *)selected_file_p, (uint32_t *)&selected_file_size));
            EXECUTION_CHECK(!recreate_file((char *)"frontend_pass.log", (char *)selected_file_p, (uint32_t *)&selected_file_size));
            printf("frontend pass: %s(%4.d bytes)\n\r", argv[1], selected_file_size);

            EXECUTION_CHECK(!analysis_pass((char *)selected_file_p, selected_file_size, (ParsedFile_t **)&directives_set, (uint32_t *)&instructions_number));
            // EXECUTION_CHECK(!save_assembly_listing((char *)"analysis_pass.log", (ParsedFile_t *)&directives_set, instructions_number));
            // printf("analysis_pass: %s(%4.d lines)\n\r", argv[1], instructions_number);
        }
        goto end_of_program;

        // // Убираем все лишнее
        // if (frontend_pass(compile_mw.firm_image.file, compile_mw.firm_image.size) == false)
        //     goto execution_error;
        // printf("frontend passes: %s(%d)\n\n\r", argv[1], compile_mw.firm_image.size);
        // // Определение адресации
        // if (analysis_pass(compile_mw.firm_image.file, compile_mw.firm_image.size, &compile_mw.preexecutable.instructions, &compile_mw.preexecutable.instructions_amount) == false)
        //     goto execution_error;
        // printf("analysis_pass: %s(%d)\n\n\r", argv[1], compile_mw.preexecutable.instructions_amount);
        // save_assembly_listing((char *)"analysis_pass.lst", compile_mw.preexecutable.instructions, compile_mw.preexecutable.instructions_amount);
        // free(compile_mw.firm_image.file);
        // backend_pass(compile_mw.preexecutable.instructions, compile_mw.preexecutable.instructions_amount, (InstructionDef *)&instruction_table, instruction_table_size);
        // printf("done...\n\r");
    }
    printf("an error has occurred...");

end_of_program:
    free((void *)selected_file_p);
    free((void *)directives_set);
    getchar();
    return 0;
}
