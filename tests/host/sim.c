/* Model of CASET / RASET / RAMWR as described in the ST7789 datasheet. */
#include "sim.h"
#include <string.h>

/* The two functions the library needs from the hardware. */
void gfx_port_cmd(uint8_t command);
void gfx_port_data(uint16_t data);

uint16_t sim_ram[SIM_RAM_H][SIM_RAM_W];
uint64_t sim_cmd_count, sim_data_count, sim_violations, sim_pixel_writes;

static uint8_t  last_cmd;
static uint8_t  param_index;
static uint16_t xs, xe = 239, ys, ye = 319;
static uint16_t cur_x, cur_y;
static uint16_t tmp[4];

void sim_reset(void)
{
	memset(sim_ram, 0, sizeof sim_ram);
	sim_cmd_count = sim_data_count = sim_violations = sim_pixel_writes = 0;
	last_cmd = 0; param_index = 0; xs = 0; xe = 239; ys = 0; ye = 319;
}

void sim_fill_ram(uint16_t color)
{
	for (int y = 0; y < SIM_RAM_H; y++)
		for (int x = 0; x < SIM_RAM_W; x++)
			sim_ram[y][x] = color;
}

void gfx_port_cmd(uint8_t command)
{
	sim_cmd_count++;
	last_cmd = command; param_index = 0;
	if (command == 0x2C) { cur_x = xs; cur_y = ys; }       /* RAMWR: counters go to (XS, YS) */
}

void gfx_port_data(uint16_t data)
{
	sim_data_count++;
	if (last_cmd == 0x2A || last_cmd == 0x2B) {             /* CASET / RASET: four 8-bit parameters */
		if (param_index < 4) tmp[param_index++] = data & 0xFF;
		if (param_index == 4) {
			uint16_t a = (uint16_t)((tmp[0] << 8) | tmp[1]), b = (uint16_t)((tmp[2] << 8) | tmp[3]);
			if (last_cmd == 0x2A) { xs = a; xe = b; } else { ys = a; ye = b; }
		}
		return;
	}
	if (last_cmd == 0x2C) {                                 /* RAMWR: one pixel */
		sim_pixel_writes++;
		int bad = (xs > xe || ys > ye || xe > 239 || ye > 319 || cur_x > xe || cur_y > ye || cur_x < xs || cur_y < ys);
		if (bad) sim_violations++;
		else sim_ram[cur_y][cur_x] = data;
		if (++cur_x > xe) { cur_x = xs; if (++cur_y > ye) cur_y = ys; }
	}
}

uint64_t sim_bus_writes(void) { return sim_cmd_count + sim_data_count; }

void sim_snapshot(uint16_t *dst)
{
	for (int y = 0; y < 240; y++)
		for (int x = 0; x < 240; x++)
			dst[y * 240 + x] = sim_ram[y][x];
}

uint64_t sim_hash_fnv1a(void)
{
	uint64_t h = 14695981039346656037ULL;                  /* FNV-1a 64-bit offset basis */
	for (int y = 0; y < 240; y++) {
		for (int x = 0; x < 240; x++) {
			uint16_t c = sim_ram[y][x];
			h ^= (uint64_t)(c >> 8);   h *= 1099511628211ULL;
			h ^= (uint64_t)(c & 0xFF); h *= 1099511628211ULL;
		}
	}
	return h;
}

void sim_save_ppm(const char *path)
{
	FILE *f = fopen(path, "wb");
	if (!f) return;
	fprintf(f, "P6\n240 240\n255\n");
	for (int y = 0; y < 240; y++) {
		for (int x = 0; x < 240; x++) {
			uint16_t c = sim_ram[y][x];
			uint8_t r = (uint8_t)(((c >> 11) & 31) * 255 / 31);
			uint8_t g = (uint8_t)(((c >> 5) & 63) * 255 / 63);
			uint8_t b = (uint8_t)((c & 31) * 255 / 31);
			fputc(r, f); fputc(g, f); fputc(b, f);
		}
	}
	fclose(f);
}
