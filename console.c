#include "console.h"
#include "reset.h"
#include "printf.h"
#include "types.h"
#include "led.h"
#define CMDSIZE 128


//function to get character from console
char (*getc)(void);

//function to put char to console. used for echoing
void (*putc)(void*, char);

void (*flush_console)(void);


int strcmp(const char *s1, const char *s2) {
  
  // Iterate as long as characters match and we haven't hit the end of s1
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  // Return the difference of the first non-matching characters.
  // Standard requires treating chars as unsigned for this comparison.
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}






//first function pointer should be getc, then putc, then flush_console
void init_console(char (*_getc)(void), void (*_putc)(void*, char), void (*_flush_console)(void)) {
  getc = _getc;
  putc = _putc;
  flush_console = _flush_console;
}

void display_banner() {
  printf("\n\r********************************\n\r");
  printf(    "*     Welcome to cobKernel     *\n\r");
  printf(    "********************************\n\r");
}

void display_prompt() {
  printf("CobKern Console: ");
}

void reboot() {
  printf("Rebooting...");
  flush_console();
  pi5_watchdog_full_reset();
}

void help() {

  printf("Cmds List\n\r");
  printf("  reboot\n\r");
  printf("  help\n\r");
    
}

void execute_cmd(char *buf) {

  if (*buf == 0)
    return;
  
  else if (!strcmp(buf, "reboot")) {
    reboot();
  }

  else if (!strcmp(buf, "help")){
    help();
  }

  else {
    printf("Invalid command: %s\n\r", buf);
    help();
    //printf("\n\r")
  }


   
}

  
void start_console() {
  //const int CMDSIZE = 128;
  char cmd_buf[CMDSIZE];
  char *cur = cmd_buf;

  display_banner();

  display_prompt();
  while (1) {
    *cur = getc();
    LED_pulse();
    
    //if newline received, send newline and carriage return, then execute command
    //currently hardcoded to work with putty which only sends '\r'
    if ((*cur == '\n') || (*cur == '\r')) {
      *cur = '\0';
      putc(NULL, '\n');
      putc(NULL, '\r');
      execute_cmd(cmd_buf);
      display_prompt();
      cur = cmd_buf;
    }

    else {
      //halt shell until newline if cmd buf is full
      if (cur != (cmd_buf + CMDSIZE -1 )) {
	putc(NULL, *cur);
	cur++;
      }
    }
  }
}



