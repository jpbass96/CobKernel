#ifndef _string_h
#define _string_h
#include "types.h"

int strlen(char *str, u32 size);
int strcmp(const char *s1, const char *s2);
void memcpy(void *src, void *dst, size_t size);
void memset(void *dst, int val, size_t size);

//split string by token. Replaces all occurences of
//token in the buffer provided with \0
char *split_str(char *buf, u32 size, char token);
//Used after split_str. Gets next string from the
//split. buf should match the buffer provided
//to split_str. end should be pointer returned by
//split_str
char *split_get_next(char *buf, char *end);
#endif