/*!
    \file    main.c
    \brief   USART transmit and receive interrupt

   \version 2026-01-09
*/

#include "gd32f30x.h"
#include <stdio.h>
#include <errno.h>
#include <sys/unistd.h>

#define ARRAYNUM(arr_nanme) (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define TRANSMIT_SIZE (ARRAYNUM(txbuffer) - 1)

uint8_t txbuffer[] = "\n\rUSART interrupt test\n\r";
uint8_t rxbuffer[32];
uint8_t tx_size = TRANSMIT_SIZE;
uint8_t rx_size = 32;
__IO uint8_t txcount = 0;
__IO uint16_t rxcount = 0;

void usart1_init(void)
{
  /* USART configuration */
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_USART1);
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
  usart_deinit(USART1);
  usart_baudrate_set(USART1, 115200U);
  usart_receive_config(USART1, USART_RECEIVE_ENABLE);
  usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
  usart_enable(USART1);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{

  /* configure USART1 */
  usart1_init();
  /* USART interrupt configuration */
  nvic_irq_enable(USART1_IRQn, 0, 0);
  /* enable USART TBE interrupt */
  usart_interrupt_enable(USART1, USART_INT_TBE);

  /* wait until USART send the transmitter_buffer */
  while (txcount < tx_size)
    ;

  while (RESET == usart_flag_get(USART1, USART_FLAG_TC))
    ;

  usart_interrupt_enable(USART1, USART_INT_RBNE);

  /* wait until USART receive the receiver_buffer */
  while (rxcount < rx_size)
  {
  }

  if (rxcount == rx_size)
  {
    printf("\n\rUSART receive successfully!\n\r");
  }

  while (1)
    ;
}

/* retarget the gcc's C library printf function to the USART */
int _write(int file, char *data, int len)
{
  if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
  {
    errno = EBADF;
    return -1;
  }

  // arbitrary timeout 1000
  for (int i = 0; i < len; i++)
  {
    usart_data_transmit(USART1, (uint8_t)data[i]);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
      ;
  }

  // return # of bytes written - as best we can tell
  return len;
}
