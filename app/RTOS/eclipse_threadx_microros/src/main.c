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

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "microros_time.h"
#include "microros_transports.h"
#include <rmw_microros/rmw_microros.h>

TX_THREAD my_thread;

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

rcl_publisher_t publisher;

#define RCCHECK(fn)                                                                \
  {                                                                                \
    rcl_ret_t temp_rc = fn;                                                        \
    if ((temp_rc != RCL_RET_OK))                                                   \
    {                                                                              \
      printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
      tx_thread_delete(NULL);                                                      \
    }                                                                              \
  }
#define RCSOFTCHECK(fn)                                                              \
  {                                                                                  \
    rcl_ret_t temp_rc = fn;                                                          \
    if ((temp_rc != RCL_RET_OK))                                                     \
    {                                                                                \
      printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
    }                                                                                \
  }

void USART1_init(void)
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

void subscription_callback(const void *msgin)
{
  const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;

  // if (msg->data == 0)
  // {
  gd_eval_led_toggle(LED2);
  // }
  // else
  // {
  //   gpio_bit_reset(GPIOF, GPIO_PIN_11);
  // }
}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  (void)last_call_time;
  (void)timer;

  static std_msgs__msg__Int32 msg = {0};

  if (RMW_RET_OK == rcl_publish(&publisher, &msg, NULL))
  {
    msg.data++;
  }
  else
  {
  }
}

void my_thread_entry(ULONG thread_input)
{
  rcl_allocator_t allocator = rcl_get_default_allocator();
  // create init_options
  rclc_support_t support;
  RCSOFTCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create nodes
  rcl_node_t node;
  RCCHECK(rclc_node_init_default(&node, "threadx_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "threadx_publisher"));

  // create subscriber
  rcl_subscription_t subscriber;
  RCSOFTCHECK(rclc_subscription_init_default(
      &subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "threadx_subscriber"));

  // create timer,
  rcl_timer_t timer;
  RCSOFTCHECK(rclc_timer_init_default2(
      &timer,
      &support,
      RCL_MS_TO_NS(1000),
      timer_callback,
      true));

  // create executor
  rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));

  std_msgs__msg__Int32 msg = {0};
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, subscription_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // while (1)
  // {
  //   ret = rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1000));
  //   tx_thread_sleep((ULONG)0.1 * TX_TIMER_TICKS_PER_SECOND);
  // }

  // (void)!rcl_publisher_fini(&publisher, &node);
  // (void)!rcl_node_fini(&node);
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
  USART1_init();
  gd_eval_led_init(LED2);
  // Configure micro-ROS Serial Agent, using USART0
  static serial_transport_args serial_comm_args = {
      .baud_rate = 115200U,
      .parity = USART_PM_NONE,
      .stop_bits = USART_STB_1BIT,
      .word_length = USART_WL_8BIT};

  rmw_uros_set_custom_transport(
      true,
      (void *)&serial_comm_args,
      microros_transport_open,
      microros_transport_close,
      microros_transport_write,
      microros_transport_read);
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
    usart_data_transmit(USART1, (uint8_t)data[i]);
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
      ;
  }

  // return # of bytes written - as best we can tell
  return len;
}