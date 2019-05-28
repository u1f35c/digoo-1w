#ifndef __TX_UART_H__
#define __TX_UART_H__

#include <stdbool.h>

#define BAUD_RATE	230400
#define UART_TX		1

void uart_tx(char c);
void uart_puts(char *s);
bool uart_lock(void);
void uart_unlock(void);
void uart_init(void);

#endif /* __TX_UART_H__ */
