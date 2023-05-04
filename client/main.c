#include <xc.h>
#include "i2c.h"
#include "main.h"
#include "can.h"
#include "external_interrupt.h"
#include "ssd_display.h"
#include "digital_keypad.h"
#include "eeprom.h"

extern bit module_on;
int select_mode_flag;
extern unsigned char can_payload[13];
unsigned char ssd[MAX_SSD_CNT];
unsigned char digit[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
int counter, node_id, mode, field;

void init_config(void)
{
	ADCON1 = 0x0F;

	/* Initialize Peripherals*/
	init_digital_keypad();
	init_ssd_control();
	init_i2c();
	init_can();

	/*Enable internal Interrupt*/
	init_external_interrupt();
	GIE = 1;

	/*Store Counter and Node ID values*/
	if (read_internal_eeprom(0x05) != 0x69)
	{
		write_internal_eeprom(0x05, 0x69);

		/*Set Node ID to 10*/
		write_internal_eeprom(0x00, 0);
		write_internal_eeprom(0x01, 10);

		/*Set Counter to Zero*/
		write_internal_eeprom(0x02, 0);
		write_internal_eeprom(0x03, 0);
	}
	else
	{
		/*Read Node ID and Counter*/
		node_id = read_internal_eeprom(0x00);				   /*Node L*/
		node_id = node_id | (read_internal_eeprom(0x01) << 8); /*Node H*/
		counter = read_internal_eeprom(0x02);				   /*Counter L*/
		counter = counter | (read_internal_eeprom(0x03) << 8); /*Counter H*/
	}
}

void main(void)
{
	init_config();

	while (1)
	{
		// Replace new Counter value if Node ID Matches
		if (can_receive())
		{
			if ((node_id == can_payload[D0]) && ((node_id >> 8) == can_payload[D1]))
			{
				module_on = TRUE;
				counter = can_payload[D2];
				counter = counter | (can_payload[D3] << 8);
				load_data_to_ssd(counter);
				select_mode_flag = RECEIVE_MODE;
			}
		}

		if (module_on)
		{
			if (select_mode_flag == SELECT_MODE)
			{
				if (mode == DATA_MODE)
				{
					ssd[0] = 0xE5; //'U'
					ssd[1] = 0x08; //'-'
					ssd[2] = FIVE; //'S'
					ssd[3] = 0xCC; //'t'
				}
				else
				{
					ssd[0] = 0xA8; //'n'
					ssd[1] = 0x08; //'-'
					ssd[2] = 0x80; //'i'
					ssd[3] = 0xE9; //'d'
				}
			}
			else if (select_mode_flag == RECEIVE_MODE)
			{
				read_key_in_receive_mode();
			}

			if (select_mode_flag != RECEIVE_MODE)
			{
				read_keys();
			}

			display(ssd);
		}
	}
}

/* delay 1ms function
void delay(unsigned short factor)
{
	unsigned short i, j;
	for (i = 0; i < factor; i++)
		for (j = 500; j--;)
			;
}*/

void read_keys(void)
{
	static int temp;
	static unsigned char key;
	key = read_digital_keypad(STATE_CHANGE);
	switch (key)
	{
	case SWITCH1:
	{
		/*Update Values*/
		if (select_mode_flag == EDIT_MODE)
		{
			if (mode == DATA_MODE)
				temp = counter;
			else
				temp = node_id;

			switch (field)
			{
			case 3:
				if (temp % 10000 >= 9000)
					temp -= 9000;
				else
					temp += 1000;
				break;
			case 2:
				if (temp % 1000 >= 900)
					temp -= 900;
				else
					temp += 100;
				break;
			case 1:
				if (temp % 100 >= 90)
					temp -= 90;
				else
					temp += 10;
				break;
			case 0:
				if (temp % 10 == 9)
					temp -= 9;
				else
					temp++;
				break;
			}

			if (mode == DATA_MODE)
				counter = temp;
			else
				node_id = temp;
		}
	}
	break;
	case SWITCH2:
	{
		if (select_mode_flag == EDIT_MODE) // Select Mode
			field = (field + 1) % 4;	   /*Change field in SSD*/
		else
			select_mode_flag = EDIT_MODE;
	}
	break;
	case SWITCH3:
	{
		if (select_mode_flag == EDIT_MODE)
		{
			if (mode == DATA_MODE)
			{
				write_internal_eeprom(0x02, counter);
				write_internal_eeprom(0x03, counter >> 8);
				can_transmit(); /*Transmit Node & Counter to Master*/
			}
			else
			{
				write_internal_eeprom(0x00, node_id);
				write_internal_eeprom(0x01, node_id >> 8);
			}

			/* Switching OFF all the SSDs*/
			turn_off_ssd();

			select_mode_flag = SELECT_MODE;
			module_on = FALSE;
		}
		else
			mode = !mode; /*Toggle between Data and Node Modes*/
	}
	break;
	}

	if (select_mode_flag == EDIT_MODE)
	{
		if (mode == DATA_MODE)
			load_data_to_ssd(counter);
		else
			load_data_to_ssd(node_id);

		switch (field)
		{
		case 3:
			ssd[0] |= DOT;
			break;
		case 2:
			ssd[1] |= DOT;
			break;
		case 1:
			ssd[2] |= DOT;
			break;
		case 0:
			ssd[3] |= DOT;
			break;
		}
	}
}

void load_data_to_ssd(int data)
{
	ssd[3] = digit[data % 10];
	data /= 10;
	ssd[2] = digit[data % 10];
	data /= 10;
	ssd[1] = digit[data % 10];
	data /= 10;
	ssd[0] = digit[data];
}

void read_key_in_receive_mode(void)
{
	switch (read_digital_keypad(STATE_CHANGE))
	{
	case SWITCH1:
		counter++;
		load_data_to_ssd(counter);
		break;
	case SWITCH2:
		if (counter > 0)
			counter--;
		load_data_to_ssd(counter);
		break;
	case SWITCH3:
	{
		write_internal_eeprom(0x02, counter);	   /*Low*/
		write_internal_eeprom(0x03, counter >> 8); /*High*/
		select_mode_flag = SELECT_MODE;
		module_on = FALSE;

		can_transmit(); /*Transmit Node & Counter to Master*/
		turn_off_ssd();
	}
	break;
	}
}

void turn_off_ssd(void)
{
	ssd[0] = BLANK;
	ssd[1] = BLANK;
	ssd[2] = BLANK;
	ssd[3] = BLANK;
	display(ssd);
	for (unsigned int wait = 50000; wait--;)
		;
}