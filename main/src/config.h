#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <cstdint>

typedef struct
{
    struct
    {
        char *file;
        uint32_t size;
    } firm_image;

} compilation_middleware_type;
extern compilation_middleware_type compilation_middleware;

#endif /* __CONFIG_H__ */