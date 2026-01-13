/*!
    \file    main.c
    \brief   UASRT receiver timeout

   \version 2025-7-31, V3.0.2, firmware for GD32F30x
*/

/*
    Copyright (c) 2025, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f30x.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>

uint8_t rxbuffer[64];
uint8_t txbuffer[64];
extern __IO uint8_t txcount;
extern __IO uint16_t rxcount;
void nvic_config(void);

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
  uint32_t i = 0, j = 0;
  nvic_config();
  USART0_init();
  printf("a usart receive timeout test example!\n");

  while (1)
  {
    if (0 == rxcount)
    {
      /* enable the USART receive interrupt */
      usart_interrupt_enable(USART0, USART_INT_RBNE);
    }
    else
    {
      /* enable the USART receive timeout and configure the time of timeout */
      usart_receiver_timeout_enable(USART0);
      usart_receiver_timeout_threshold_config(USART0, 115200 * 3);

      while (RESET == usart_flag_get(USART0, USART_FLAG_RT))
        ;
      for (i = 0; i < rxcount; i++)
      {
        txbuffer[i] = rxbuffer[j++];
      }
      /* disable the USART receive interrupt and enable the USART transmit interrupt */
      usart_interrupt_disable(USART0, USART_INT_RBNE);
      usart_interrupt_enable(USART0, USART_INT_TBE);

      while (txcount < rxcount)
        ;
      usart_flag_clear(USART0, USART_FLAG_RT);
      txcount = 0;
      rxcount = 0;
      i = 0;
      j = 0;
    }
  }
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
  nvic_irq_enable(USART0_IRQn, 0, 1);
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