/**
 * @file gfx_port.h
 * @brief The only connection between the library and the hardware.
 *
 * The library needs exactly two operations on the display controller:
 *
 *  - gfx_port_cmd(c)   write a COMMAND byte  (A0 = 0, e.g. FMC_BANK2->REG)
 *  - gfx_port_data(d)  write a DATA word     (A0 = 1, e.g. FMC_BANK2->RAM)
 *
 * Implement both once in your application. With the STM32F723E-DISCO board
 * and the FMC in 16-bit mode they are one-liners:
 *
 * @code
 * void gfx_port_cmd(uint8_t c)   { FMC_BANK2_WriteReg(c);  }
 * void gfx_port_data(uint16_t d) { FMC_BANK2_WriteData(d); }
 * @endcode
 *
 * @par What travels through gfx_port_data()
 * The same function carries two kinds of payload, selected by the command
 * that was sent last:
 *  - after CASET (0x2A) or RASET (0x2B): four 8-bit parameters, in the low
 *    byte of @p data (the high byte is always zero);
 *  - after RAMWR (0x2C): one 16-bit RGB565 pixel per call.
 *
 * A port that is wider or narrower than 16 bits must track the last command
 * and split or merge words accordingly. See the porting guide in docs/.
 *
 * @par What is not in scope
 * Controller power-up and register initialisation (sleep out, pixel format,
 * memory access control, display on) belong to your board support code.
 */
#ifndef GFX_PORT_H_
#define GFX_PORT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write one command byte to the controller (A0 = 0).
 * @param command Command opcode, for example 0x2A (CASET).
 */
void gfx_port_cmd(uint8_t command);

/**
 * @brief Write one data word to the controller (A0 = 1).
 * @param data 8-bit parameter (low byte) or a 16-bit RGB565 pixel.
 */
void gfx_port_data(uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* GFX_PORT_H_ */
