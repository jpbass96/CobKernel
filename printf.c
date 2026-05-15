/*
File: printf.c

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "printf.h"
#include "types.h"
#include "arm.h"

//global kernel console
struct char_device kernel_console;

//Need a larger buffer for the long string than for 32-bit values
#ifdef PRINTF_LONG_SUPPORT
#define ITOA_BF_SIZE 24
static void uli2a(unsigned long num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned long d=1;
    while (num/d >= base)
        d*=base;         
    while (d!=0) {
        long dgt = num / d;
        num%=d;
        d/=base;
        if (n || dgt>0|| d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }

static void li2a (long num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    uli2a(num,10,0,bf);
    }

long strtol(const char *str, int base) {
    long val;
    u8 negative = 0;
    val = 0;
    while (*str) {
        if (base == 10 && *str == '-') {
            negative = 1;
        }

        //handle regular digit. Shift current val by one digit and add new digit
        else if ((*str >= '0') && (*str <= '9')) {
            val = val*base + (*str - '0');
        }

        //handle lowercase alphabet digit if base > 10
        else if (base > 10 && (*str >= 'a') && (*str < ((base-10) +  'a'))) {
            val = val*base + (*str - 'a' + 10);
        }

        //handle uppercase alphabet digit if base > 10
        else if (base > 10 && (*str >= 'A') && (*str < ((base-10) + 'A'))) {
            val = val*base + (*str - 'A' + 10);
        }
        else {
            break;
        }
        str++;
    } 

    val = negative ? val*-1 : val;
    return val;
}

#else
#define ITOA_BF_SIZE 12
#endif

static void ui2a(unsigned int num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
        d*=base;        
    while (d!=0) {
        int dgt = num / d;
        num%= d;
        d/=base;
        if (n || dgt>0 || d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }

static void i2a (int num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    ui2a(num,10,0,bf);
    }

static int a2d(char ch)
    {
    if (ch>='0' && ch<='9') 
        return ch-'0';
    else if (ch>='a' && ch<='f')
        return ch-'a'+10;
    else if (ch>='A' && ch<='F')
        return ch-'A'+10;
    else return -1;
    }

static char a2i(char ch, char** src,int base,int* nump)
    {
    char* p= *src;
    int num=0;
    int digit;
    while ((digit=a2d(ch))>=0) {
        if (digit>base) break;
        num=num*base+digit;
        ch=*p++;
        }
    *src=p;
    *nump=num;
    return ch;
    }

static void putchw(void* putp,putcf putf,int n, char z, char* bf)
    {
    char fc=z? '0' : ' ';
    char ch;
    char* p=bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0) 
        putf(putp,fc);
    while ((ch= *bf++))
        putf(putp,ch);
    }

void tfp_format(struct char_device *dev, char *fmt, va_list va)
    {

    char bf[ITOA_BF_SIZE];
    
    char ch;
    while ((ch=*(fmt++))) {
        if (ch!='%') 
            dev->putf(dev->putp,ch);
        else {
            char lz=0;
#ifdef  PRINTF_LONG_SUPPORT
            char lng=0;
#endif
            int w=0;
            ch=*(fmt++);
            if (ch=='0') {
                ch=*(fmt++);
                lz=1;
                }
            if (ch>='0' && ch<='9') {
                ch=a2i(ch,&fmt,10,&w);
                }
#ifdef  PRINTF_LONG_SUPPORT
            if (ch=='l') {
                ch=*(fmt++);
                lng=1;
            }
#endif
            switch (ch) {
                case 0: 
                    goto abort;
                case 'u' : {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long),10,0,bf);
                    else
#endif
                    ui2a(va_arg(va, unsigned int),10,0,bf);
                    putchw(dev->putp,dev->putf,w,lz,bf);
                    break;
                    }
                case 'd' :  {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        li2a(va_arg(va, unsigned long),bf);
                    else
#endif
                    i2a(va_arg(va, int),bf);
                    putchw(dev->putp,dev->putf,w,lz,bf);
                    break;
                    }
                case 'x': case 'X' : 
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a(va_arg(va, unsigned long),16,(ch=='X'),bf);
                    else
#endif
                    ui2a(va_arg(va, unsigned int),16,(ch=='X'),bf);
                    putchw(dev->putp,dev->putf,w,lz,bf);
                    break;
                case 'c' : 
                    dev->putf(dev->putp,(char)(va_arg(va, int)));
                    break;
                case 's' : 
                    putchw(dev->putp,dev->putf,w,0,va_arg(va, char*));
                    break;
                case '%' :
                    dev->putf(dev->putp,ch);
                default:
                    break;
                }
            }
        }
    abort:;

   
    }

void init_printf(struct char_device* dev, void* putp,void (*putf) (void*,char)) {
    dev->putf=putf;
    dev->putp=putp;
    dev->thread_safe = FALSE;
}

void init_printf_threadsafe(struct char_device* dev, void* putp,void (*putf) (void*,char)) {
    init_printf(dev, putp, putf);
    arm64_init_semaphore(&(dev->sem));
    dev->thread_safe = TRUE;
}

void kinit_printf(void* putp,void (*putf) (void*,char)) {
    init_printf_threadsafe(&kernel_console, putp, putf);
}

void tfp_printf(struct char_device *dev, char *fmt, ...) {
    if (dev->thread_safe)
        arm64_take_semaphore_exclusive(&(dev->sem));

    va_list va;
    va_start(va,fmt);
    tfp_format(dev,fmt,va);
    va_end(va);

    if (dev->thread_safe)
        arm64_put_semaphore_exclusive(&(dev->sem));
}



static void putcp(void* p,char c)
    {
    *(*((char**)p))++ = c;
    }

void tfp_sprintf(char* s,char *fmt, ...) {
    struct char_device strdev;
    init_printf(&strdev, &s, putcp);
    va_list va;
    va_start(va,fmt);
    tfp_format(&strdev, fmt, va);
    putcp(&s,0);
    va_end(va);
}
