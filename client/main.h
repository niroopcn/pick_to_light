#ifndef MAIN_H
#define MAIN_H

void delay(unsigned short factor);
void read_keys(void);
void load_data_to_ssd(int data);
void read_key_in_receive_mode(void);
void turn_off_ssd(void);

#define _XTAL_FREQ 20000000

/* Defines the data */
#define TRUE 1
#define FALSE 0

/*Define MODES*/
#define NODE_MODE 0
#define DATA_MODE 1

/*Define Mode Flag*/
#define SELECT_MODE 0
#define EDIT_MODE 1
#define RECEIVE_MODE 2

#endif
