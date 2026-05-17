#include "file_sys.hpp"

/// @brief Read the file
/// @param file_name name of the file afterward return interpreted file
/// @param ret_file_size returned file size
/// @return condition od reading file
bool open_file(char **file_name, uint32_t *ret_file_size)
{
    FILE *file = fopen(*file_name, "rb");
    size_t bytes_read, file_size;
    char *tmp_file;

    if (file != NULL && file_name && ret_file_size)
    {
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);

        rewind(file);
        tmp_file = (char *)malloc(sizeof(char) * (file_size + 1));
        if (tmp_file != NULL)
        {
            bytes_read = fread(tmp_file, 1, file_size, file);
            tmp_file[bytes_read] = '\0';
            {
                *file_name = tmp_file;
                *ret_file_size = file_size;
            }
            fclose(file);
            return (file_size == bytes_read);
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