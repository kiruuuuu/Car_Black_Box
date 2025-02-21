#ifndef SCI_H
#define SCI_H

#define RX_PIN					TRISC7
#define TX_PIN					TRISC6

void init_uart(void);
void putch(unsigned char byte); //to transmit the one byte of the data
int puts(const char *s);   //To transmit a string
unsigned char getch(void);  //To recevie one byte of data
unsigned char getche(void); // it recevie 1 byte and transmit 1 byte
unsigned char getche(void); 

#endif
