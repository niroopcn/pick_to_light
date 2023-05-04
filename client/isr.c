#include <xc.h>
#include "main.h"
#include "ssd_display.h"

extern unsigned char ssd[MAX_SSD_CNT];
bit module_on;

void interrupt isr(void)
{
	if (INT0F == 1)
	{
		module_on = !module_on;

		if (!module_on)
			turn_off_ssd();

		INT0F = 0;
	}
}
