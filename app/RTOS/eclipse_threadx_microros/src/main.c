/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group,
   byte pool, and block pool.
   Source: https://github.com/eclipse-threadx/threadx/blob/master/ports/cortex_m4/gnu/example_build/sample_threadx.c
*/

#include "tx_api.h"

#include <sys/errno.h>
#include <stdio.h>
#include <unistd.h>
#include "gd32f30x.h"
#include "gd32f3_eval.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "microros_transports.h"
#include <rmw_microros/rmw_microros.h>

#define DEMO_STACK_SIZE 2048
#define DEMO_BYTE_POOL_SIZE 9120
#define DEMO_BLOCK_POOL_SIZE 100
#define DEMO_QUEUE_SIZE 100

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

/* Define the ThreadX object control blocks...  */

TX_THREAD thread_0;
TX_EVENT_FLAGS_GROUP event_flags_0;
TX_BYTE_POOL byte_pool_0;
TX_BLOCK_POOL block_pool_0;
UCHAR memory_area[DEMO_BYTE_POOL_SIZE];

/* Define thread prototypes.  */

void thread_0_entry(ULONG thread_input);

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

void subscription_callback(const void *msgin)
{
  const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;

  if (msg->data == 0)
  {
    gpio_bit_set(GPIOF, GPIO_PIN_11);
  }
  else
  {
    gpio_bit_reset(GPIOF, GPIO_PIN_11);
  }
}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  (void)last_call_time;
  (void)timer;

  static std_msgs__msg__Int32 msg = {0};

  if (RMW_RET_OK == rcl_publish(&publisher, &msg, NULL))
  {
    // printf("Sent: %ld\n", msg.data);
    msg.data++;
  }
  else
  {
    // printf("Failed to send\n");
  }
}

/* Define main entry point.  */

int main()
{

  USART0_init();
  printf("Starting ThreadX + microROS code sample...\n");

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
  /* Enter the ThreadX kernel.  */
  tx_kernel_enter();
}

/* Define what the initial system looks like.  */

void tx_application_define(void *first_unused_memory)
{

  CHAR *pointer = TX_NULL;

  /* Create a byte memory pool from which to allocate the thread stacks.  */
  tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);

  /* Put system definition stuff in here, e.g. thread creates and other assorted
     create information.  */

  /* Allocate the stack for thread 0.  */
  tx_byte_allocate(&byte_pool_0, (VOID **)&pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

  /* Create the main thread.  */
  tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,
                   pointer, DEMO_STACK_SIZE,
                   4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
}

/* Define the test threads.  */

void thread_0_entry(ULONG thread_input)
{
  rcl_ret_t res;
  // UINT status;
  rcl_allocator_t allocator = rcl_get_default_allocator();
  // create init_options
  rclc_support_t support;
  res = rclc_support_init(&support, 0, NULL, &allocator);

  // create nodes
  rcl_node_t node;
  res = rclc_node_init_default(&node, "threadx_node", "", &support);

  // create publisher
  res = rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "threadx_publisher");

  // create subscriber
  rcl_subscription_t subscriber;
  res = rclc_subscription_init_default(
      &subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "threadx_subscriber");

  // create timer,
  rcl_timer_t timer;
  rclc_timer_init_default2(
      &timer,
      &support,
      RCL_MS_TO_NS(1000),
      timer_callback,
      true);

  // create executor
  rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
  rclc_executor_init(&executor, &support.context, 2, &allocator);

  std_msgs__msg__Int32 msg = {0};
  rclc_executor_add_subscription(&executor, &subscriber, &msg, subscription_callback, ON_NEW_DATA);
  rclc_executor_add_timer(&executor, &timer);

  /* This thread simply sits in while-forever-sleep loop.  */
  while (1)
  {
  }
}

// /* retarget the gcc's C library printf function to the USART */
// int _write(int file, char *data, int len)
// {
//   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
//   {
//     errno = EBADF;
//     return -1;
//   }

//   // arbitrary timeout 1000
//   for (int i = 0; i < len; i++)
//   {
//     usart_data_transmit(USART0, (uint8_t)data[i]);
//     while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
//       ;
//   }

//   // return # of bytes written - as best we can tell
//   return len;
// }