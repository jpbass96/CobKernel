#ifndef _console_h
#define _console_h
#include "types.h"

#define VA_ARGS_TO_CMDTYPE_ARRAY(...) ((cmd_arg_type[]){##__VA_ARGS__ })
#define REGISTER_COMMAND(_name, _help, _func, _numargs, ...) \
    static struct console_command cmd_##_func \
    __attribute__((used, section(".cmd_array"))) = \
    { \
        .help = _help, \
        .name = _name, \
        .cmd_ptr = _func, \
        .numargs = _numargs, \
        .arg_typearr = {__VA_ARGS__} \
    };
    
typedef enum arg_type {
    CMD_END,
    CMD_INT,
    CMD_LONG,
    CMD_PTR,
    CMD_STR,
} cmd_arg_type;

//statically allocate help, name, and arg arrays for now
//TODO: Update command registry to add an init function that
//can dynamically allocate the command or add extra arguments.
struct console_command {
    char help[256];
    char name[64];
    int (*cmd_ptr)(void *params);
    u8 numargs;
    cmd_arg_type arg_typearr[8];
};

int strcmp(const char *s1, const char *s2);
void init_console(char (*_getc)(void), void (*putc)(void*, char), void (*flush_console)(void));
int reboot(void* params);
int help(void *params);
void execute_cmd(char *buf);

void display_banner();

void start_console(); 

#endif
