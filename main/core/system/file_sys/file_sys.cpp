#include "file_sys.hpp"

bool open_file(char **file_name, uint32_t *ret_file_size)
{
    if (*file_name && file_name && ret_file_size)
    {
        FILE *file = fopen(*file_name, "rb");
        *file_name = NULL;

        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            *ret_file_size = ftell(file);
            rewind(file);

            *file_name = (char *)malloc(sizeof(char) * (*ret_file_size + 1));
            if (*file_name != NULL)
            {
                size_t bytes_read = fread(*file_name, 1, *ret_file_size, file);
                (*file_name)[bytes_read] = '\0', fclose(file);
                return (*ret_file_size == bytes_read);
            }
        }
    }
    return 0;
}

bool recreate_file(char *file_name, char *data_ptr, uint32_t *ret_file_size)
{
    if (file_name != NULL && data_ptr != NULL && ret_file_size != NULL)
    {
        uint32_t bytes_to_write = *ret_file_size;
        FILE *file = fopen(file_name, "wb");

        if ((file != NULL) && (bytes_to_write != 0))
        {
            *ret_file_size = fwrite(data_ptr, 1, bytes_to_write, file);
            fclose(file);
            return (*ret_file_size == bytes_to_write);
        }
        *ret_file_size = 0;
    }
    return 0;
}