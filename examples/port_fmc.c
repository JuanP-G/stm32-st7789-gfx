/*
 * Port for a 16-bit FMC (NOR/SRAM mode) with FMC_A0 wired to the controller's D/CX pin.
 *
 * With a 16-bit memory width the FMC drives FMC_A0 from address bit 1, so a
 * write to the bank base has A0 = 0 (command) and a write to base + 2 has
 * A0 = 1 (data).
 *
 *     FMC_NE1  0x60000000    FMC_NE3  0x68000000
 *     FMC_NE2  0x64000000    FMC_NE4  0x6C000000
 */
#include <stdint.h>

#include "gfx_port.h"

#define LCD_BASE 0x64000000u                              /* FMC_NE2: adjust to your chip select */
#define LCD_REG  (*(volatile uint16_t *)(LCD_BASE))       /* A0 = 0: command */
#define LCD_RAM  (*(volatile uint16_t *)(LCD_BASE + 2u))  /* A0 = 1: data    */

void gfx_port_cmd(uint8_t command)
{
    LCD_REG = command;
}

void gfx_port_data(uint16_t data)
{
    LCD_RAM = data;
}
