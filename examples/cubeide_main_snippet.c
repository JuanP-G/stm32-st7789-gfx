/*
 * Excerpt of a STM32CubeIDE main.c. It is not built here because it needs the
 * HAL and the code CubeMX generates for your board.
 *
 * Steps in CubeMX: enable FMC in NOR/SRAM mode with a 16-bit data bus, the chip
 * select you wired and address line A0, then generate the project and add this
 * library's include/ and src/ folders (see the getting-started guide).
 */

/* USER CODE BEGIN Includes */
#include "gfx_core.h"
#include "gfx_demo.h"
#include "gfx_selftest.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PFP */
extern void lcd_init(void);   /* your controller power-up: sleep out, pixel format 0x3A = 0x55, MADCTL, display on */
/* USER CODE END PFP */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_FMC_Init();

    /* USER CODE BEGIN 2 */
    lcd_init();

    /* 1. Wiring check: step through the nine test patterns. */
    for (int page = 0; page < gfx_selftest_page_count(); page++) {
        gfx_selftest_page(page);
        HAL_Delay(2000);
    }
    /* USER CODE END 2 */

    while (1) {
        /* USER CODE BEGIN 3 */
        /* 2. Then the demo pages, three seconds each. */
        for (int page = 0; page < gfx_demo_page_count(); page++) {
            gfx_demo_page(page);
            HAL_Delay(3000);
        }
        /* USER CODE END 3 */
    }
}
