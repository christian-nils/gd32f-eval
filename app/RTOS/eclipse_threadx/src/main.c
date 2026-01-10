/*!
    \file    main.c
    \brief   USART printf

   \version 2026-01-09
*/

#include "gd32f30x.h"
#include <unistd.h>
#include <sys/errno.h>
#include "gd32f3_eval.h"
#include <stdio.h>
#include "tx_api.h"
#include "cmsis_utils.h"

TX_THREAD my_thread;

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

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

void my_thread_entry(ULONG thread_input)
{
  USART0_init();

  printf("\n\rWelcome to this ThreadX thread...\n\r");
  int val = 0;
  while (1)
  {
    printf("%d\n", val++);
    /* Sleep for 1 second. */
    tx_thread_sleep(1UL * TX_TIMER_TICKS_PER_SECOND);
  }
}

void tx_application_define(void *first_unused_memory)
{

  systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

  /* Create my_thread! */
  tx_thread_create(&my_thread, "My Thread", my_thread_entry, 0,
                   azure_thread_stack, AZURE_THREAD_STACK_SIZE,
                   AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
                   TX_NO_TIME_SLICE, TX_AUTO_START);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
  /* Enter the ThreadX kernel. */
  tx_kernel_enter();
  return 0;
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
