#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"

#define TX_BUF_SIZE 64
#define TX_BUF_MASK (TX_BUF_SIZE - 1)
#define RX_BUF_SIZE 32
#define RX_BUF_MASK (RX_BUF_SIZE - 1)
#define RX_LINES 8
#define RX_LINES_MASK (RX_LINES - 1)

//#define UART_ECHO


static volatile char tx_buf[TX_BUF_SIZE];
static volatile uint8_t tx_write;
static volatile uint8_t tx_read;

static volatile char rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_write;
static volatile uint8_t rx_read;

static volatile char rx_lines[RX_LINES][RX_BUF_SIZE + 1];
static volatile uint8_t rx_lines_write;
static volatile uint8_t rx_lines_read;


static char uart_getc();


void uart_init()
{
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	// flush receive buffer
	volatile uint8_t tmp;
	do {
		tmp = UDR0;
	} while (UCSR0A & (1 << RXC0));

	// enable receive interrupt
	UCSR0B |= (1 << RXCIE0);
}


ISR(USART0_RX_vect)
{
	uint8_t slot = (rx_write + 1) & RX_BUF_MASK;
	uint8_t data = UDR0;

#ifdef UART_ECHO
	uart_putc(data);
	if (data == '\r') {
		uart_putc('\n');
	}

#endif

	if (data == 0x00 || data == 0xff) {
		// skip *obvious* nonsense
	}
	else if (data == '\r' || data == '\n') {
		// move line to line buffer
		if (rx_write == rx_read) {
			// ignore empty line, do nothing
		}
		else {
			uint8_t lineslot = (rx_lines_write + 1) & RX_LINES_MASK;
			if (lineslot == rx_lines_read) {
				// line buffer overflow, discard line (i.e. the whole rx buf)
				rx_write = rx_read = 0;
			}
			else {
				// copy line into line buffer
				uint8_t i = 0;
				char c;
				while ((c = uart_getc()) && i < RX_BUF_SIZE) {
					rx_lines[lineslot][i++] = c;
				}
				rx_lines[lineslot][i] = '\0';
				rx_lines_write = lineslot;
			}
		}
	}
	else if (slot == rx_read) {
		// buffer overflow
	}
	else {
		// copy char into rx buffer
		rx_buf[slot] = data;
		rx_write = slot;
	}
}


ISR(USART0_UDRE_vect)
{
	if (tx_write != tx_read) {
		tx_read = (tx_read + 1) & TX_BUF_MASK;
		UDR0 = tx_buf[tx_read];
	}
	else {
		// nothing left to transmit => disable interrupt
		UCSR0B &= ~(1 << UDRIE0);
	}
}

static inline char uart_getc()
{
	if (rx_write == rx_read) {
		return '\0';
	}
	rx_read = (rx_read + 1) & RX_BUF_MASK;
	return rx_buf[rx_read];
}


char *uart_getline()
{
	static char line[RX_BUF_SIZE + 1];
	if (rx_lines_write == rx_lines_read) {
		// no line available
		return NULL;
	}
	uint8_t lineslot = (rx_lines_read + 1) & RX_LINES_MASK;
	strcpy(line, (const char *)rx_lines[lineslot]);
	rx_lines_read = lineslot;
	return line;
}



inline void uart_putc(char c)
{
	uint8_t slot = (tx_write + 1) & TX_BUF_MASK;
	while (slot == tx_read) {
		// wait until there's space available
	}
	tx_buf[slot] = c;
	tx_write = slot;
	// enable transmit interrupt
	UCSR0B |= (1 << UDRIE0);
}


inline void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}


void uart_printf(const char *fmt, ...)
{
	static char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	uart_puts(buf);
}
