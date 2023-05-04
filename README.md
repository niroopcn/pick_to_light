# pick_to_light

PIC18F4580 on rhydoLABZ Board

Peripherals used: Seven Segment Display, External Interrupt, Digital Keypad & Internal EEPROM.

Protocols Used: UART & CAN

Description: Pick2Light is a client server concept used in warehouse management. The idea is to direct the floor operator to the glowing light and pick the displayed amount of components from the bin. This helps the industries to increase the efficiency, instead of a paper based approach of sorting, ordering and picking components. By integrating the Pick 2 Light in every bin, a good number of automation can be achieved. This project is concentrated on the implementation of the node (client) which interacts with the server whenever required.

CAN protocol is used to connect one server node to ‘n’ no. of client nodes having different unique IDs. Data can be broadcasted to the client node via the server node using CAN protocol and data to the server node is provided via a Host console (Laptop) using UART. The respective nodes with matching ID will receive the data and communication is established.

![IMG_20230504_172121](https://user-images.githubusercontent.com/70747194/236258163-73bd92f9-9aa6-4749-bc97-fcdb55839b1a.jpg)

![IMG_20230504_165822](https://user-images.githubusercontent.com/70747194/236258659-b24c9265-f4b0-4e66-be8c-a4facdadb934.jpg)
