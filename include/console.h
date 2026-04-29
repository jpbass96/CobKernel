#ifndef _console_h
#define _console_h

int strcmp(const char *s1, const char *s2);
void init_console(char (*_getc)(void), void (*putc)(void*, char), void (*flush_console)(void));
void reboot();
void help();
void execute_cmd(char *buf);

void display_banner();

void start_console(); 

#endif
