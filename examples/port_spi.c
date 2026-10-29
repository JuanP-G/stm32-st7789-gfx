/*
 * Port for a 4-wire SPI panel (SCK, MOSI, CS and a D/C pin).
 *
 * gfx_port_data() carries 8-bit parameters after CASET and RASET and 16-bit
 * pixels after RAMWR, so the port remembers the last command: one byte for
 * parameters, two bytes with the high byte first for pixels.
 *
 * Provide the three hooks below with your HAL. This version favours clarity.
 * For speed, keep CS low for the whole burst and use the fastest SPI clock the
 * panel accepts.
 */
#include <stdint.h>

#include "gfx_port.h"

#define GFX_CMD_RAMWR 0x2Cu

extern void lcd_spi_write(const uint8_t *buf, uint16_t len);
extern void lcd_dc(int level);   /* 0 = command, 1 = data */
extern void lcd_cs(int level);   /* 0 = selected */

static uint8_t last_cmd;

void gfx_port_cmd(uint8_t command)
{
    last_cmd = command;
    lcd_dc(0);
    lcd_cs(0);
    lcd_spi_write(&command, 1);
    lcd_cs(1);
}

void gfx_port_data(uint16_t data)
{
    lcd_dc(1);
    lcd_cs(0);
    if (last_cmd == GFX_CMD_RAMWR) {
        uint8_t px[2];
        px[0] = (uint8_t)(data >> 8);
        px[1] = (uint8_t)data;
        lcd_spi_write(px, 2);
    } else {
        uint8_t p = (uint8_t)data;
        lcd_spi_write(&p, 1);
    }
    lcd_cs(1);
}
