/*
 * Port for an 8080-style 8-bit parallel bus (D[7:0], WR strobe and a D/C pin).
 *
 * Same rule as SPI: one byte for parameters, two bytes (high first) for pixels.
 * Provide the three hooks below with your GPIO code or a faster port register.
 */
#include <stdint.h>

#include "gfx_port.h"

#define GFX_CMD_RAMWR 0x2Cu

extern void lcd_bus_put(uint8_t value);   /* put value on D[7:0] */
extern void lcd_wr_pulse(void);           /* WR low, then high: the controller latches on the rising edge */
extern void lcd_dc(int level);            /* 0 = command, 1 = data */

static uint8_t last_cmd;

static void bus_write8(uint8_t value)
{
    lcd_bus_put(value);
    lcd_wr_pulse();
}

void gfx_port_cmd(uint8_t command)
{
    last_cmd = command;
    lcd_dc(0);
    bus_write8(command);
}

void gfx_port_data(uint16_t data)
{
    lcd_dc(1);
    if (last_cmd == GFX_CMD_RAMWR) {
        bus_write8((uint8_t)(data >> 8));
        bus_write8((uint8_t)data);
    } else {
        bus_write8((uint8_t)data);
    }
}
