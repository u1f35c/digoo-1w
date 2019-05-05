#ifndef __W1_H__
#define __W1_H__

#include <stdbool.h>

#define W1_PIN 2	/* Pin 2 on Port B */

uint8_t w1_crc(uint8_t *buf, uint8_t len);
void w1_write(uint8_t val);
bool w1_read_bit();
uint8_t w1_read_byte();
void w1_read(uint8_t *buf, uint8_t len);
bool w1_reset(bool nowait);
void w1_init(void);

#endif /* __W1_H__ */
