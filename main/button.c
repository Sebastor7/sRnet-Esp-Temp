#include "button.h"
#include "esp_timer.h"
#include <stdio.h>

// Queue to handle button press events
static QueueHandle_t button_evt_queue = NULL;  // Use QueueHandle_t instead of xQueueHandle

// Variables to track button state and timing
static uint64_t button_press_time = 0;
static bool button_pressed = false;

// Define a static variable for the callback function
static button_callback_t button_callback = NULL;

// Interrupt service routine (ISR) for button press
static void IRAM_ATTR button_isr_handler(void* arg) {
    int gpio_num = (int)arg;
    // Send GPIO number to queue to notify task of button press/release
    xQueueSendFromISR(button_evt_queue, &gpio_num, NULL);
}

// Task to handle button press detection and timing
static void button_task(void* arg) {
    int gpio_num;
    while (1) {
        // Wait for an event from ISR
        if (xQueueReceive(button_evt_queue, &gpio_num, portMAX_DELAY)) {
            // Check if button is pressed or released
            if (gpio_get_level(BUTTON_GPIO) == 0 && !button_pressed) {
                // Button pressed
                button_pressed = true;
                button_press_time = esp_timer_get_time();  // Record time in microseconds
                printf("Button pressed\n");

            } else if (gpio_get_level(BUTTON_GPIO) == 1 && button_pressed) {
                // Button released
                button_pressed = false;
                uint64_t press_duration = (esp_timer_get_time() - button_press_time) / 1000;  // Duration in ms

                // Debounce: Ignore if press duration is too short
                if (press_duration < DEBOUNCE_TIME) {
                    continue;
                }

                // Determine if it was a short or long press
                if (press_duration < SHORT_PRESS_THRESHOLD) {
                    printf("Short press detected: %llu ms\n", press_duration);
                    if (button_callback) {
                        button_callback(BUTTON_SHORT_PRESS);  // Call the callback with short press event
                    }
                } else {
                    printf("Long press detected: %llu ms\n", press_duration);
                    if (button_callback) {
                        button_callback(BUTTON_LONG_PRESS);  // Call the callback with long press event
                    }
                }

                printf("Button released after: %llu ms\n", press_duration);
                if (button_callback) {
                    button_callback(BUTTON_RELEASE);  // Call the callback with release event
                }
            }
        }
    }
}

// Public function to initialize button handling
void button_init(void) {
    // Configure the button GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;  // Interrupt on both edges (press and release)
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);  // Select GPIO pin
    io_conf.mode = GPIO_MODE_INPUT;  // Set as input
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Create a queue to handle button events from the ISR
    button_evt_queue = xQueueCreate(10, sizeof(int));  // Use QueueHandle_t instead of xQueueHandle

    // Install ISR service
    gpio_install_isr_service(0);  // Use 0 or specific interrupt flags

    // Attach ISR handler to the button GPIO
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void*)BUTTON_GPIO);

    // Create a task to handle button press detection and timing
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
}

// Public function to register a callback
void button_register_callback(button_callback_t cb) {
    button_callback = cb;
}

