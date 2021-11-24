#include "stm8s.h"

#define PUTCHAR_PROTOTYPE void putchar (char c)

/**
  * @brief Retargets the C library printf function to the UART.
  * @param c Character to send
  * @retval char Character sent
  */
PUTCHAR_PROTOTYPE
{
  /* Write a character to the UART1 */
  UART1_SendData8(c);
  /* Loop until the end of transmission */
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);

  //return (c);
}

void debug_init(void)
{
  UART1_DeInit();
  
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  UART1_ITConfig(UART1_IT_RXNE, ENABLE);
}

void send_str(char c[]) {
	char i;
	//char len;
	//len = sizeof(c);
	//len = strlen(c);
	i = 0;
	while (c[i] > 0) {
		UART1_SendData8(c[i]);
		while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
		i++;
	}
}
