#include <stdio.h>
#include <string.h>
#include <rt_sys.h>
#include <rt_misc.h>
#include "drv/uart.h"

#if (__ARMCC_VERSION<6000000)
#pragma import(__use_no_semihosting)
#pragma import(__use_no_semihosting_swi)
struct __FILE { int handle;};
#else
__asm(".global __use_no_semihosting");
__asm(".global __use_no_semihosting_swi");
struct FILE { int handle;};
#endif


//#define STDIO

/* Standard IO device handles. */
#define STDIN   0x8001
#define STDOUT  0x8002
#define STDERR  0x8003

/* Standard IO device name defines. */
const char __stdin_name[]  = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";


FILE __stdout;
FILE __stdin;


handle_t dbgHandle=NULL;
int fputc(int c, FILE *f)
{
    return (uart_rw(dbgHandle, (U8*)&c, 1, 1));
}


int fgetc(FILE *f)
{
    int c;
    uart_rw(dbgHandle, (U8*)&c, 1, 0);
    return c;
}


int ferror(FILE *f)
{
    return EOF;
}


#ifdef STDIO
int sendchar(int c)
{
    return 0;
}

int getkey() {
    return 0;
}
#endif


void _ttywrch (int c)
{
#ifdef STDIO
    sendchar(c);
#else
    uart_rw(dbgHandle, (U8*)&c, 1, 1);
#endif
}


FILEHANDLE _sys_open (const char *name, int openmode)
{
    /* Register standard Input Output devices. */
    if (strcmp(name, "STDIN") == 0) {
        return (STDIN);
    }
    if (strcmp(name, "STDOUT") == 0) {
        return (STDOUT);
    }
    if (strcmp(name, "STDERR") == 0) {
        return (STDERR);
    }
    return (-1);
}



int _sys_close (FILEHANDLE fh)
{
    if (fh > 0x8000) {
        return (0);
    }
    return (-1);
}



int _sys_write (FILEHANDLE fh, const unsigned char *buf, unsigned int len, int mode)
{
#ifdef STDIO
    if (fh == STDOUT) {
        /* Standard Output device. */
        for (  ; len; len--) {
            sendchar (*buf++);
        }
        return (0);
    }
#endif
    if (fh > 0x8000) {
        return (-1);
    }
    return (-1);
}


int _sys_read (FILEHANDLE fh, unsigned char *buf, unsigned int len, int mode)
{
#ifdef STDIO
    if (fh == STDIN) {
    /* Standard Input device. */
        for (  ; len; len--) {
            *buf++ = getkey ();
        }
        return (0);
    }
#endif
    if (fh > 0x8000) {
        return (-1);
    }
    return (-1);
}


int _sys_istty (FILEHANDLE fh)
{
    if (fh > 0x8000) {
        return (1);
    }
    return (0);
}


int _sys_seek (FILEHANDLE fh, long pos)
{
    if (fh > 0x8000) {
        return (-1);
    }
    return (-1);
}


int _sys_ensure (FILEHANDLE fh)
{
    if (fh > 0x8000) {
        return (-1);
    }
    return (-1);
}


long _sys_flen (FILEHANDLE fh)
{
    if (fh > 0x8000) {
        return (0);
    }
    return (-1);
}


int _sys_tmpnam (char *name, int sig, unsigned maxlen)
{
    return (1);
}



char *_sys_command_string (char *cmd, int len)
{
    return (cmd);
}


void _sys_exit (int return_code) {
    /* Endless loop. */
    while (1);
}
