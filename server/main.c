#include <xc.h>
#include "main.h"
#include "uart.h"
#include "can.h"

void init_config(void)
{
	init_uart();
	init_can();

	puts("PICK TO LIGHT PROJECT:\n\rPress any key to continue\n\r");
	getch();
	RCIF = 0;

	/* Enabling peripheral interrupt */
	PEIE = 1;
	/* Enabling global interrupt */
	GIE = 1;
}

unsigned char ch;
extern unsigned char can_payload[13];
char node[4], data[4];
char node_low, node_high, data_low, data_high;

void main()
{
	init_config();

	char iter = 0, once = 1;
	int node_id, counter;

	while (1)
	{
		if (once && (iter == 0))
		{
			once = 0;
			puts("\n\rEnter Node ID (4 Digits): ");
		}
		else if (once && (iter == 4))
		{
			once = 0;
			puts("\n\rEnter Data (4 Digits): ");
		}

		if (ch != '\0')
		{
			if (iter < 4)
			{
				putch(ch);
				node[iter] = ch - '0';
			}
			else if (iter < 8)
			{
				putch(ch);
				data[iter - 4] = ch - '0';
				if (iter == 7)
				{
					node_id = (node[0] * 1000) + (node[1] * 100) + (node[2] * 10) + node[3];
					counter = (data[0] * 1000) + (data[1] * 100) + (data[2] * 10) + data[3];

					node_low = node_id;
					node_high = node_id >> 8;
					data_low = counter;
					data_high = counter >> 8;

					puts("\n\r");
					can_transmit();
				}
			}
			if (iter == 7 || iter == 3)
				once = 1;
			iter = (iter + 1) % 8;

			ch = '\0';
		}
		else if (can_receive())
		{
			iter = 0;
			once = 1;

			node_id = can_payload[D0];
			node_id = node_id | (can_payload[D1] << 8);
			counter = can_payload[D2];
			counter = counter | (can_payload[D3] << 8);

			puts("\n\rData Received!!\n\r");

			puts("Node ID: ");
			putch(((node_id / 1000) % 10) + '0');
			putch(((node_id / 100) % 10) + '0');
			putch(((node_id / 10) % 10) + '0');
			putch((node_id % 10) + '0');
			puts("\n\r");

			puts("Counter: ");
			putch(((counter / 1000) % 10) + '0');
			putch(((counter / 100) % 10) + '0');
			putch(((counter / 10) % 10) + '0');
			putch((counter % 10) + '0');
			puts("\n\r");
		}
	}
}
