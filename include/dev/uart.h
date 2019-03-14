#ifndef __UART_H
#define __UART_H

#include <sys/buf.h>

int uart_init (void *dev);
int uart_shut (void *dev);
void uart_isr ();
int uart_ioctl (void *dev, int cmd, void *args);
int uart_getchar ();
void uart_putchar (int ch);
int uart_put (void *dev, int c);
int uart_get (void *dev, int *c);

#endif
