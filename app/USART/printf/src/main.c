/*!
    \file    main.c
    \brief   USART printf

   \version 2026-01-09
*/

#include "gd32f30x.h"
#include <unistd.h>
#include <sys/errno.h>
#include <stdio.h>

void USART0_init(void)
{
  /* USART configuration */
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_USART0);
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
  usart_deinit(USART0);
  usart_baudrate_set(USART0, 115200U);
  usart_receive_config(USART0, USART_RECEIVE_ENABLE);
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
  usart_enable(USART0);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
  USART0_init();

  printf("\n\ran USART transmit test example!\n\r");
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
    usart_data_transmit(USART0, (uint8_t)data[i]);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
      ;
  }

  // return # of bytes written - as best we can tell
  return len;
}
