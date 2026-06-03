#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <cstdint>
#include "main.hpp"

typedef struct
{
    struct
    {
        char *file;
        uint32_t size;
    } firm_image;

    struct
    {
        ParsedFile_t *instructions;
        uint32_t instructions_amount;
    } preexecutable;

} compilation_middleware_type;
extern compilation_middleware_type compilation_middleware;

#endif /* __CONFIG_H__ */