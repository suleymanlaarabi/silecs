#include "stdio_devtool_utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

const char *stdio_devtool_trim_whitespace(const char *str)
{
    if (!str)
        return NULL;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return NULL;

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    static char buffer[1024];
    size_t len = (size_t)(end - str + 1);
    if (len >= sizeof(buffer))
        len = sizeof(buffer) - 1;

    memcpy(buffer, str, len);
    buffer[len] = '\0';

    return buffer;
}

void stdio_devtool_clear_screen(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}
