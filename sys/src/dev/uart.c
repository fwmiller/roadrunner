#include <dev/uart.h>
#include <string.h>
#include <sys.h>
#include <sys/intr.h>

/* COM port I/O base addresses */
#define COM1_BASE	((short) 0x3f8)
#define COM2_BASE	((short) 0x2f8)
#define COM3_BASE	((short) 0x3e8)
#define COM4_BASE	((short) 0x2e8)

/* UART register offsets */
#define COM_RBR		0	/* Receive buffer ready */
#define COM_TBH		0	/* Transmit buffer holding */
#define COM_DLL		0	/* Divisor latch lo */
#define COM_IER		1	/* Interrupt enable */
#define COM_DLH		1	/* Divisor latch hi */
#define COM_IIR		2	/* Interrupt id */
#define COM_FCR		2	/* FIFO control */
#define COM_LCR		3	/* Line control */
#define COM_MCR		4	/* Modem control */
#define COM_LSR		5	/* Line status */
#define COM_MSR		6	/* Modem status */

#define RX_FIFO_LEN	8

/* Receive FIFO */
static char rx_fifo[RX_FIFO_LEN];
static int rx_fifo_head, rx_fifo_tail, rx_fifo_len;

int uart_init(void *dev)
{
	memset(rx_fifo, 0, RX_FIFO_LEN);
	rx_fifo_head = 0;
	rx_fifo_tail = 0;
	rx_fifo_len = 0;

	outb(COM1_BASE + COM_IER, 0x00);	/* Disable interrupts */
	outb(COM1_BASE + COM_LCR, 0x80);	/* Enable DLAB */
	outb(COM1_BASE + COM_DLL, 0x01);	/* 115200 baud rate lo */
	outb(COM1_BASE + COM_DLH, 0x00);	/* 115200 baud rate hi */
	outb(COM1_BASE + COM_LCR, 0x03);	/* 8 bits,no parity,1 stop */
	outb(COM1_BASE + COM_FCR, 0x00);	/* Disable FIFOs */
	outb(COM1_BASE + COM_IER, 0x01);	/* Enable IRQs */

	return 0;
}

int uart_shut(void *dev)
{
	return 0;
}

void uart_isr()
{
	char ch;

	if (inb(COM1_BASE + COM_LSR) & 0x01) {
		ch = inb(COM1_BASE + COM_RBR);

		disable;
		if (rx_fifo_len < RX_FIFO_LEN) {
			rx_fifo[rx_fifo_tail] = ch;
			rx_fifo_tail = (rx_fifo_tail + 1) % RX_FIFO_LEN;
			rx_fifo_len++;
		}
		enable;
	}
}

int uart_ioctl(void *dev, int cmd, void *args)
{
	return 0;
}

int uart_getchar()
{
	int ch = 0;

	while (1) {
		disable;
		if (rx_fifo_len == 0) {
			enable;
			continue;
		}
		ch = (int)rx_fifo[rx_fifo_head];
		rx_fifo_head = (rx_fifo_head + 1) % RX_FIFO_LEN;
		rx_fifo_len--;

		enable;
		break;
	}
	return ch;
}

void uart_putchar(int ch)
{
	outb(COM1_BASE + COM_TBH, ch);
}

int uart_put(void *dev, int c)
{
	uart_putchar(c);
	return 0;
}

int uart_get(void *dev, int *c)
{
	*c = uart_getchar();
	return 0;
}
