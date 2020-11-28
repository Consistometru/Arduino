#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include <driver/adc.h>
#include "sdkconfig.h"


#define BLINK_GPIO 2 /*CONFIG_BLINK_GPIO */
#define TOUCH_PAD 0
#define OUTPUT_GPIO 13

uint16_t prag = 300;
bool actiune = 0;
int tic = 1000;

static void touch_read_task()
{
 uint16_t touch_value;
  /* Wait touch sensor init done */
  vTaskDelay(100 / portTICK_RATE_MS);
    printf("Touch Sensor read, the output format is: \nTouchpad num:[raw data]\n\n");
  while (1) {
      int hall = hall_sensor_read();
      printf("Hall value= %d\n", hall);
      touch_pad_read(TOUCH_PAD, &touch_value);
      printf("T%d: [%d] ", TOUCH_PAD, touch_value);
      printf("\n");
         if (touch_value < prag ) {
         printf("Execute\n");
         gpio_set_level(BLINK_GPIO, !actiune);
         gpio_set_level(OUTPUT_GPIO, actiune);
         actiune = !actiune;
         } 
     vTaskDelay(tic / portTICK_PERIOD_MS);
    }
}


void app_main() {
 printf("Hello world!\n");

 gpio_pad_select_gpio(BLINK_GPIO);
 gpio_pad_select_gpio(OUTPUT_GPIO);

    /* Set the GPIO as a push/pull output */
 gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
 gpio_set_direction(OUTPUT_GPIO, GPIO_MODE_OUTPUT);
#if 1
 gpio_set_level(BLINK_GPIO, actiune);
 gpio_set_level(OUTPUT_GPIO, !actiune);
#endif
 touch_pad_init();
 touch_pad_config(TOUCH_PAD, prag);

 adc1_config_width(ADC_WIDTH_BIT_12);
 

  xTaskCreate(&touch_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);

}