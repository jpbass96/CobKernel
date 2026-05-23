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
#include "work_queue.h"
#include "primes.h"
#include "time.h"
#include "string.h"
#include "bcm2712_temp.h"
#include "math.h"

#define CMDSIZE 256
#define BKSPC 0x8
#define ESC 0x1B
#define DEL 0x7F
#define CSI_LEFT 'D'
#define CSI_RIGHT 'C'

extern struct work_queue_entry *queues[3];
extern void *__cmd_start;
extern void *__cmd_end;

enum ansi_state {
  ANSI_COMMAND,
  ANSI_CSI
};

//function to get character from console
char (*getc)(void);
//function to put char to console. used for echoing
void (*putc)(void*, char);
void (*flush_console)(void);

//first function pointer should be getc, then putc, then flush_console
void init_console(char (*_getc)(void), void (*_putc)(void*, char), void (*_flush_console)(void)) {
  getc = _getc;
  putc = _putc;
  flush_console = _flush_console;
  
}

void display_prompt() {
  printf("CobKern Husk: ");
}

int reboot(void *params) {
  printf("Rebooting...");
  flush_console();
  pi5_watchdog_full_reset();
  return 0;
}
REGISTER_COMMAND("reboot", "no help", reboot, 0)

//inline int _calculate_primes_task_handler(void *params) { return calculate_primes(*(u64*)params, *(u64*)((void*)params + sizeof(u64)));}

int prime_multicore_test(void *_params) {

  struct __attribute__((packed)) prime_params {
    u64 primes;
    u32 num_cores;
    u64 minimum_work_size;
  };

  struct prime_params *pparams = (struct prime_params *)_params;

  u64 params[6];
  u64 start, stop;
  u64 primes_per_core;
  
  // this could truncate number so be careful with you parameters!
  primes_per_core = pparams->primes / pparams->num_cores;

  LOG_INFO("Executing multicore prime calculation test. Primes Per Core : %ld, cores: %d, work_size: %ld\n\r", primes_per_core, pparams->num_cores, pparams->minimum_work_size);
  
  start = get_kernel_time_us();
  for (u64 i=0; i < primes_per_core*pparams->num_cores; i+= pparams->minimum_work_size*pparams->num_cores) {
    for (u8 qidx = 0; qidx < pparams->num_cores; qidx++) {
      params[qidx*2] = i + (qidx *  pparams->minimum_work_size);
      params[qidx*2+1]  = i + ((qidx + 1) *  pparams->minimum_work_size);
      LOG_DEBUG("Putting work into q %d\n\r", qidx);
      QUEUE_WORK(queues[qidx], calculate_primes, (void*)(&params[qidx*2]));
    }
    for (u8 qidx = 0; qidx < pparams->num_cores; qidx++) {
      LOG_DEBUG("Waitng for work done on q %d\n\r", qidx);
      wait_work_done(queues[qidx]);
    }
  }
  stop = get_kernel_time_us();
  LOG_INFO("Operation took %ld us\n\r", stop - start);
  return 0;
}
REGISTER_COMMAND("prime_multicore_test", "nohelp", prime_multicore_test, 3, CMD_LONG, CMD_INT, CMD_LONG)

int sqrt_test(void *params) {
  u64 val;
  val = *(u64*)params;

  LOG_INFO("Val is %ld\n\r", val);
  val = sqrt(val);
  LOG_INFO("sqrt of Val is %ld\n\r", val);
  return 0;
}
REGISTER_COMMAND("sqrt_test", "nohelp", sqrt_test, 1, CMD_LONG)

int sem_test(void *params) {
  
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
  return 0;
}
REGISTER_COMMAND("sem_test", "nohelp", sem_test, 0)

int heap_test(void *params) {
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

  return 0;
}
REGISTER_COMMAND("heap_test", "nohelp", heap_test, 0)

int print_long_test(void *params) {
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
  return 0;
}

REGISTER_COMMAND("print_long_test", "nohelp", print_long_test, 0)

int get_temp(void *params) {
  int val;
  int status;
  status = bcm2712_get_temp(&val);
  if (status == 0)
    LOG_INFO("Temperature is currently %d mC\n\r", val);
  else
    LOG_ERROR("Could not read temperature\n\r");

  return 0;
}

REGISTER_COMMAND("get_temp", "nohelp", get_temp, 0)

int help(void *params) {
  struct console_command *cmd;
  struct console_command *cmdstart = (struct console_command *)&__cmd_start;
  struct console_command *cmdend =  (struct console_command *)&__cmd_end;

  printf("Cmds List\n\r");
  for (cmd = cmdstart; cmd < cmdend; cmd++) {
    printf("  %s: %s\n\r", cmd->name, cmd->help);
  }
  return 0;
}
REGISTER_COMMAND("help", "nohelp", help, 0)

int print_pcie_cfg(void *params) {

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

  return 0;
}

REGISTER_COMMAND("print_pcie_cfg", "nohelp", print_pcie_cfg, 0)

char *get_next_arg(char *buf, char *end) {
  
  //find the first position where *buf is NULL indicating a split
  while ((buf < (end-1)) && (*buf != '\0')) {
    buf++;
  }

  //find the next non-zero position in case there were multiple spaces in the oken
  while ((buf < (end-1)) && (*buf == '\0')) {
    buf++;
  }

  //if the next non-zero argument at or 1 away from the end, then we cannot parse anymore arguments. return NULL
  if (buf >= (end-1)) //if we are at the very end, there are no more args)
    return NULL;

  //return current position as it holds the non-zero character
  return buf;

}

char *split_args(char *buf) {
  char *cur = buf;

  while ((*cur != '\0')) {
    if (*cur == ' ') {
      *cur = '\0';
    }
    cur++;
  }
  return cur;
}

void _execute_cmd(struct console_command *cmd, char *buf, char *end) {

  char *nxt;
  void *params;
  void *curparam;
  u64 arg;
  nxt = buf;
  u8 argsize;
  
  params = kmalloc(CMDSIZE);
  curparam = params;
  for (int i = 0; i < cmd->numargs; i++) {
    nxt = split_get_next(nxt, end);
    if (nxt == NULL) {
      LOG_ERROR("invalid number of arguments for cmd %s\n\r", cmd->name);
      return;
    }
    switch (cmd->arg_typearr[i]) {
    
      case CMD_LONG:
      case CMD_PTR:
        argsize = sizeof(long);
        arg = strtol(nxt, 10);
        *(unsigned long*)curparam = arg;
        break;
      case CMD_INT:
        argsize = sizeof(int);
        arg = strtol(nxt, 10);
        *(unsigned int*)curparam = (unsigned int)arg;
        break;
      case CMD_STR:
        LOG_ERROR("CMD_STR NOT YET SUPPORTED\n\r");
        return;
        break;
      default:
        LOG_ERROR("Invalid command argument\n\r");
        return;
        break;
    }
    curparam += argsize;
  }
  cmd->cmd_ptr(params);
  kfree(params);
}

void execute_cmd(char *buf) {
  struct console_command *cmd, *cmdstart, *cmdend;
  cmdstart = (struct console_command *)&__cmd_start;
  cmdend =  (struct console_command *)&__cmd_end;
  char *end;
  
  if (*buf == 0)
    return;

  //cmd = (struct console_command *)&__cmd_start;
  end = split_str(buf, CMDSIZE, ' ');
  LOG_INFO("comparing: %s\n\r", buf);
  for (cmd = cmdstart; cmd < cmdend; cmd++) {
    if (!strcmp(buf, cmd->name)) {
      
      LOG_INFO("name: %s\n\r", cmd->name);
      _execute_cmd(cmd, buf, end);
      return;
    }
  }

  LOG_ERROR("Invalid command %s\n\r", buf);
  help(NULL);
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



