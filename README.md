# stm32_wave_creator
Use the STM32 processor to generate PWM waves, sine waves, cosine waves, triangle waves, zigzag waves, and more with adjustable duty cycle and frequency
This code is a comprehensive example of an embedded systems project, specifically for a STM32F103RCT6 microcontroller, which is part of the STM32F103x6/x8 family. It includes initialization routines for PWM output, OLED display, key input, and a watchdog timer. Here's a multifaceted evaluation of the code:

1. Functionality and Features:
PWM Generation: The code initializes Timer 2 to generate PWM signals on four channels, which is a common requirement in many industrial and hobbyist projects.
OLED Display: It integrates an OLED display for real-time feedback, which is useful for user interfaces in embedded systems.
User Input: The code includes a routine for reading from four buttons, allowing for user interaction with the device.
Watchdog Timer: The inclusion of a watchdog timer is a good practice for resetting the microcontroller in case of a software failure or lockup.
External Interrupts: The code uses external interrupts to handle signals from a rotary encoder, which is a common method for implementing user input for adjusting parameters like frequency or duty cycle.

2. Efficiency:
The code seems to be efficient in terms of resource usage, as it reuses variables and structures where possible.
The use of a 1ms interrupt service routine (ISR) for handling timing-related tasks is efficient and ensures that the main loop is not bogged down with timing logic.

3. Reliability:
The code includes a watchdog timer initialization, which is a good reliability feature to prevent system hangs.
The use of external interrupts with debounce logic for the rotary encoder is also a good practice to ensure reliable input handling.
