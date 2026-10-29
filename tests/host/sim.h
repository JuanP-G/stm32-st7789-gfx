/* Model of the ST7789 controller, used to test the library on a PC. */
#ifndef SIM_H_
#define SIM_H_

#include <stdint.h>
#include <stdio.h>

#define SIM_RAM_W 240
#define SIM_RAM_H 320

extern uint16_t sim_ram[SIM_RAM_H][SIM_RAM_W];
extern uint64_t sim_cmd_count;       /* gfx_port_cmd() calls */
extern uint64_t sim_data_count;      /* gfx_port_data() calls */
extern uint64_t sim_violations;      /* pixels sent outside the window or outside the RAM */
extern uint64_t sim_pixel_writes;    /* data words received after RAMWR */

void sim_reset(void);
void sim_fill_ram(uint16_t color);
void sim_save_ppm(const char *path);               /* the visible 240x240 area */
void sim_snapshot(uint16_t *dst240x240);

/* Total bus writes so far (commands + data). */
uint64_t sim_bus_writes(void);

/* FNV-1a (64 bit) over the visible 240x240 area, row by row, each pixel as
 * two bytes (high byte first), so the result does not depend on endianness. */
uint64_t sim_hash_fnv1a(void);

#endif
