#ifndef BUTTON_H
#define BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// Button press thresholds (in milliseconds)
#define SHORT_PRESS_THRESHOLD 1000  // 1 second
#define DEBOUNCE_TIME 50  // 50 milliseconds

// GPIO pin for the button (can be customized per application)
#define BUTTON_GPIO GPIO_NUM_0  // Example GPIO for button

// Define possible button events
typedef enum {
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS,
    BUTTON_RELEASE
} button_event_t;

// Define the type for the callback function
typedef void (*button_callback_t)(button_event_t event);

// Public function to initialize button handling
void button_init(void);

// Public function to register the callback
void button_register_callback(button_callback_t cb);

#endif // BUTTON_H
