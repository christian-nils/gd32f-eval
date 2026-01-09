/*!
    \file    gd32f3_eval.h
    \brief   definitions for the gd32f3-eval board from OLIMEX

*/

#ifndef GD32F3_EVAL_H
#define GD32F3_EVAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gd32f30x.h"

  /* exported types */
  typedef enum
  {
    LED1 = 0,
    LED2 = 1
  } led_typedef_enum;

/* eval board low layer led */
#define LEDn 4U

#define LED1_PIN GPIO_PIN_5
#define LED1_GPIO_PORT GPIOA
#define LED1_GPIO_CLK RCU_GPIOA

#define LED2_PIN GPIO_PIN_1
#define LED2_GPIO_PORT GPIOA
#define LED2_GPIO_CLK RCU_GPIOA

#define COMn 2U

#define EVAL_COM0 USART0
#define EVAL_COM0_CLK RCU_USART0
#define EVAL_COM0_TX_PIN GPIO_PIN_9
#define EVAL_COM0_RX_PIN GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT GPIOA
#define EVAL_COM0_GPIO_CLK RCU_GPIOA

#define EVAL_COM1 USART1
#define EVAL_COM1_CLK RCU_USART1
#define EVAL_COM1_TX_PIN GPIO_PIN_2
#define EVAL_COM1_RX_PIN GPIO_PIN_3
#define EVAL_COM1_GPIO_PORT GPIOA
#define EVAL_COM1_GPIO_CLK RCU_GPIOA

  /* function declarations */
  /* configure led GPIO */
  void gd_eval_led_init(led_typedef_enum lednum);
  /* turn on selected led */
  void gd_eval_led_on(led_typedef_enum lednum);
  /* turn off selected led */
  void gd_eval_led_off(led_typedef_enum lednum);
  /* toggle the selected led */
  void gd_eval_led_toggle(led_typedef_enum lednum);
  /* configure COM port */
  void gd_eval_com_init(uint32_t com);

#ifdef __cplusplus
}
#endif

#endif /* GD32F307C_EVAL_H */
