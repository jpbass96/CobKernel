#include "console.h"
#include "reset.h"
#include "printf.h"
#include "types.h"
#include "led.h"
#include "rp1_pcie.h"
#include "util.h"
#include "types.h"
#include "arm.h"
#include "malloc.h"
#define CMDSIZE 256
#define BKSPC 0x8
#define ESC 0x1B
#define DEL 0x7F
#define CSI_LEFT 'D'
#define CSI_RIGHT 'C'

enum ansi_state {
  ANSI_COMMAND,
  ANSI_CSI
};

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

void display_prompt() {
  printf("CobKern Husk: ");
}

void reboot() {
  printf("Rebooting...");
  flush_console();
  pi5_watchdog_full_reset();
}

void sem_test() {
  
  arm64_sem sem;
  u32 status;

  printf("sem address is 0x%lx\n\r", &sem);
  printf("Sem value is 0x%x\n\r", sem);
  
  printf("Trying to init sem\n\r");
  status = arm64_init_semaphore(&sem);
  printf("status after init is 0x%x\n\r", status);
  
  printf("Trying to take sem\n\r");

  arm64_take_semaphore_exclusive(&sem);

  printf("trying to release sem\n\r");
  arm64_put_semaphore_exclusive(&sem);

  printf("Sem test complete\n\r");

}

void heap_test() {
  void *addr1, *addr2, *addr3, *addr4, *addr5;
  printf("Trying to allocate 64KB\n\r");
  addr1 = kmalloc(0x10000);
  printf("allocated address was 0x%lx\n\r", addr1);
  printf("\n\r");
  printf("Trying to allocate an additional 65KB\n\r");
  addr2 = kmalloc(0x10400);
  printf("allocated address was 0x%lx\n\r", addr2);
  printf("\n\r");
  printf("Trying to allocate an additional 256KB\n\r");
  addr3 = kmalloc(0x40000);
  printf("allocated address was 0x%lx\n\r", addr3);
  printf("\n\r");
  printf("Freeing addr2, and allocating 32KB in its place\n\r");
  kfree(addr2);
  addr2 = kmalloc(0x8000);
  printf("allocated address was 0x%lx\n\r", addr2);
  printf("\n\r");

  printf("Trying to allocate an additional 980.8125MB\n\r");
  addr4 = kmalloc(0x3D4D0000);
  printf("allocated address was 0x%lx\n\r", addr4);
  printf("\n\r");

  printf("Trying to allocate an additional 64KB\n\r");
  addr5 = kmalloc(0x400);
  printf("allocated address was 0x%lx\n\r", addr5);
  printf("\n\r");

  printf("Trying to allocate an additional 980.8125MB. Expecting this to fail\n\r");
  printf("allocated address was 0x%lx\n\r", kmalloc(0x3D4D0000));
  printf("\n\r");


  printf("Freeing large allocation\n\r");
  kfree(addr4);

  printf("Trying to allocate an additional 64KB\n\r");
  addr4 = kmalloc(0x400);
  printf("allocated address was 0x%lx\n\r", addr4);
  printf("\n\r");


  printf("Freeing NULL ptr\n\r");
  kfree(NULL);
  printf("\n\r");

  printf("Freeing pointer in range but not to start of page\n\r");
  kfree(addr2 + 8);
  printf("\n\r");

  printf("Doing double free\n\r");
  kfree(addr3);
  kfree(addr3);
  printf("\n\r");

  printf("Freeing memory outside of heap range\n\r");
  kfree((void*)0x80000000);
  printf("\n\r");
}

void print_long_test() {
  u32 val = 0xff00ff00;
  u64 val2 = 0xf123456789abcdefULL;

  u32 val3 = 0x0f00ff00;
  u64 val4 = 0x0123456789abcdefULL;
  
  printf("val deciamal Expected: 4278255360 or -16711936. Actual %d\n\r", val);
  printf("val deciamal Expected: 0xff00ff00. Actual %x\n\r", val);

  printf("val2 deciamal Expected: ?? or -1070935975390360081. Actual %ld\n\r", val2);
  printf("val2 deciamal Expected: 0xf123456789abcdef. Actual %lx\n\r", val2);


  printf("val3 deciamal Expected: 251723520. Actual %d\n\r", val3);
  printf("val3 deciamal Expected: 0x0f00ff00. Actual %x\n\r", val3);

  printf("val4 deciamal Expected: 81985529216486895. Actual %ld\n\r", val4);
  printf("val4 deciamal Expected: 0x0123456789abcdef. Actual %lx\n\r", val4);
}

void help() {

  printf("Cmds List\n\r");
  printf("  reboot\n\r");
  printf("  get_pcie_windows\n\r");
  printf("  print_pcie_cfg\n\r");
  printf("  sem_test\n\r");
  printf("  print_long_test\n\r");
  printf("  heap_test\n\r");
  printf("  help\n\r");
    
}

void print_pcie_cfg() {

  for (int bus = 0; bus < 256; bus++) {
    for (int dev = 0; dev < 32; dev++) {
      u32 vid;

      vid = rp1_pcie_cfg_read(bus, dev << 3, 0);
      if (vid != 0xFFFFFFFF) {
	printf("detected vid at bus %d dev %d\n\r", bus, dev);
	printf("printing config space \n\r");
	for (int i = 0; i < 256; i+=4) {
	  u32 dat;
	  
	  dat = rp1_pcie_cfg_read(bus, dev << 3, i);
    
	  printf("PCI Cfg Addr 0x%x: 0x%x\n\r", i, dat);
	}
      }
    }
  }
}

void execute_cmd(char *buf) {

  if (*buf == 0)
    return;
  
  else if (!strcmp(buf, "reboot")) {
    reboot();
  }

  else if (!strcmp(buf, "get_pcie_windows")) {
    rp1_read_pcie_windows();
  }

  else if (!strcmp(buf, "print_pcie_cfg")) {
    print_pcie_cfg();
  }

  else if (!strcmp(buf, "sem_test")) {
    sem_test();
  }

  else if (!strcmp(buf, "print_long_test")) {
    print_long_test();
  }

  else if (!strcmp(buf, "heap_test")) {
    heap_test();
  }
  
  else if (!strcmp(buf, "help")){
    help();
  }

  else {
    char *tmp;
    printf("Invalid command: %s\n\r", buf);
    help();

    tmp = buf;
    printf("command hex encoding\n\r");
    while (*tmp != '\0') {
      printf("0x%x ", *tmp);
      tmp++;
    }
    printf("\n\r");
  }
}

//handles newline and returns new current command position
static inline char *_handle_newline(char *cmd, char* cur) {
  *cur = '\0';
  putc(NULL, '\n');
  putc(NULL, '\r');
  cur = cmd;
  return cur;
}

static inline char *_handle_del(char *cmd, char *cur) {
  return cur;
}

//handles backspace and returns new current command position
static inline char *_handle_bkspc(char *cmd, char*cur) {
  //set previous character to NULL in cmd buffer
  putc(NULL, BKSPC);
  putc(NULL, ' ');
  putc(NULL, BKSPC);
  *(cur) = 0;

  //remain at current position and overwrite next character
  return cur-1;
}

//handles escape command code
char *_handle_esc(char *cmd, char*cur) {
  char ansi_buf[32];
  char *_cur = ansi_buf;
  
  enum ansi_state decode_state = ANSI_COMMAND;
  //emit ESC byte back
  putc(NULL, ESC);
  while (_cur != (cmd + CMDSIZE - 1)) {
    *_cur = getc();
    
    switch  (decode_state) {
      //try to decode command
      case ANSI_COMMAND:
        // Move state machine to CSI decode state
        if (*_cur == '[') {
          decode_state = ANSI_CSI;
          putc(NULL, '[');
        }
        //unsupported command found. simply return current position and do not modify cmd buffer
        else {
          //emit null byte to cancel command
          putc(NULL, '\0');
          return cur;
        }
        break;
      case ANSI_CSI:
        putc(NULL, *_cur);
        //check if current byte is a command code. Otherwise keep collecting data in ANSI_CSI state
        //Does not currently correctly decode the ';'
        if ((*_cur >= 0x40) && (*_cur <= 0x7E)) {
          char ansi_cmd;
          u32 arg;

          //save cmd value and set _cur byte back to 0 so strtol finds null byte when decoding argument
          ansi_cmd = *_cur;
          *_cur = 0;

          //cur is beginning of escape code, cur+1 is CSI command, cur+2 is start of optional argument
          //If no argument provided, default to 1
          if (_cur - cur > 2) {
            arg = strtol(cur+2, 16); 
          }
          else {
            //arg defaults to 1
            arg = 1;
          }
          
          //Decode and execute cursor movement command using decoded argument
          if (ansi_cmd == CSI_LEFT) {
            cur = max(cmd, (cur - (arg)));
            return cur;
          }
          else if (ansi_cmd == CSI_RIGHT) {
            cur = min(cmd + CMDSIZE - 1, (cur + (arg)));
            return cur;
          }
        }

        //if ; is detected, emit NULL to cancel command. Not currently supported
        else if (*_cur == ';') {
          putc(NULL, '\0');
        }
        break;
    }
    ++_cur;
  }

  return cur;
}

void start_console() {
  //const int CMDSIZE = 128;
  char cmd_buf[CMDSIZE];
  char *cur = cmd_buf;
  char next;

  printf("Now Entering Cobkernel Early Husk\n\r");
  display_prompt();
  while (1) {
    next = getc();
    LED_pulse();
    
    switch (next) {
      //if newline received, send newline and carriage return, then execute command
     //currently hardcoded to work with putty which only sends '\r'
      case '\n':
      case '\r':
        cur = _handle_newline(cmd_buf, cur);
        execute_cmd(cmd_buf);
        display_prompt();
        break;
    
      case BKSPC:
        cur = _handle_bkspc(cmd_buf, cur);
        break;
      case ESC:
        cur = _handle_esc(cmd_buf, cur);
        break;

      case DEL:
        cur = _handle_del(cmd_buf, cur);
        break;

      default:
        //halt shell until newline if cmd buf is full
        if (cur != (cmd_buf + CMDSIZE -1 )) {
          *cur = next;
	        putc(NULL, *cur);
	        cur++;
        }
    } 
  }
}



