#define BAUD 57600
#define UBRR (F_CPU/16/BAUD-1)

#define FRAME_ERROR (1<<FE0)
#define DATA_OVERRUN (1<<DOR0)
#define PARITY_ERROR (1<<UPE0)

void UART_Init(void);
unsigned char UART_Transmit(unsigned char *);
unsigned char UART_Receive(unsigned char *);
unsigned char UART_Transmit_Array(unsigned char *);
unsigned char UART_Receive_Array(unsigned char *);

