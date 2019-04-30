#ifndef __TX_UART_H__
#define __TX_UART_H__

#define BAUD_RATE	115200
#define UART_TX		2

void uart_tx(char c);
void uart_puts(char *s);
void uart_init(void);

#endif /* __TX_UART_H__ */
