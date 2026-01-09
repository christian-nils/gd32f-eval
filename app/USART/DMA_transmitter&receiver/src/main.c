/*!
    \file    main.c
    \brief   transmit/receive data using DMA interrupt

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
#include <errno.h>
#include <sys/unistd.h>

#define ARRAYNUM(arr_name) (uint32_t)(sizeof(arr_name) / sizeof(*(arr_name)))
#define USART1_DATA_ADDRESS ((uint32_t)&USART_DATA(USART1))

__IO FlagStatus g_transfer_complete = RESET;
uint8_t rx_buffer[10];
uint8_t tx_buffer[] = "\n\rUSART DMA receive and transmit example, please input 10 bytes:\n\r";

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
  /* initialize USART */
  usart1_init();

  printf("starting program...\n");

  dma_parameter_struct dma_init_struct;
  /* enable DMA0 */
  rcu_periph_clock_enable(RCU_DMA0);

  /* deinitialize DMA channel3(USART1 tx) */
  dma_deinit(DMA0, DMA_CH3);
  dma_struct_para_init(&dma_init_struct);
  dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_addr = (uint32_t)tx_buffer;
  dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
  dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
  dma_init_struct.number = ARRAYNUM(tx_buffer);
  dma_init_struct.periph_addr = USART1_DATA_ADDRESS;
  dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
  dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
  dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
  dma_init(DMA0, DMA_CH3, &dma_init_struct);
  /* configure DMA mode */
  dma_circulation_disable(DMA0, DMA_CH3);
  /* enable DMA channel3 */
  dma_channel_enable(DMA0, DMA_CH3);

  /* enable USART DMA for transmission */
  usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);

  /* wait DMA channel transfer complete */
  while (RESET == dma_flag_get(DMA0, DMA_CH3, DMA_INTF_FTFIF))
  {
  }

  while (1)
  {
    /* deinitialize DMA channel4 (USART1 rx) */
    dma_deinit(DMA0, DMA_CH4);
    usart_flag_clear(USART1, USART_FLAG_RBNE);
    usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rx_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 10;
    dma_init_struct.periph_addr = USART1_DATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    /* enable DMA channel4 */
    dma_channel_enable(DMA0, DMA_CH4);

    /* wait DMA channel transfer complete */
    while (RESET == dma_flag_get(DMA0, DMA_CH4, DMA_INTF_FTFIF))
    {
    }

    usart_dma_receive_config(USART1, USART_RECEIVE_DMA_DISABLE);
    printf("\n\r%s\n\r", rx_buffer);
  }
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
