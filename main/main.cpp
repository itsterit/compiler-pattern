#include "config.hpp"

int main(int argc, char *argv[])
{
    char *selected_file_p = argv[1];
    uint32_t selected_file_size = 0;
    ParsedFile_t *directives_set = 0;
    uint32_t instructions_number = 0;

    while ((argc == 2) && (argv))
    {
        printf("\n\rstarting...\n\r");
        {
            EXECUTION_CHECK(!open_file(&selected_file_p, &selected_file_size));
            printf("selected file: %s(%-4.d bytes)\n\r", argv[1], selected_file_size);

            EXECUTION_CHECK(!frontend_pass(selected_file_p, &selected_file_size));
            EXECUTION_CHECK(!recreate_file((char *)"frontend_pass.log", selected_file_p, &selected_file_size));
            printf("frontend pass: %s(%-4.d bytes)\n\r", argv[1], selected_file_size);

            EXECUTION_CHECK(!analysis_pass(selected_file_p, selected_file_size, &directives_set, &instructions_number));
            EXECUTION_CHECK(!save_assembly_listing((char *)"analysis_pass.log", directives_set, instructions_number));
            printf("analysis_pass: %s(%-4.d lines)\n\r", argv[1], instructions_number);

            EXECUTION_CHECK(!backend_pass(directives_set, instructions_number, (InstructionDef *)&instruction_table, instruction_table_size));
            EXECUTION_CHECK(!save_assembly_listing((char *)"backend_pass.log", directives_set, instructions_number));
        }
        printf("done...\n\r");
        goto end_of_program;
    }
    printf("an error has occurred...");

end_of_program:
    free((void *)selected_file_p);
    free((void *)directives_set);
    getchar();
    return 0;
}
