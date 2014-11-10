micro-p
=======

This is a project that was completed in 2013 as part of the course ECSE 426 - Microprocessor Systems at McGill University.

To view the associated project report: https://dl.dropboxusercontent.com/u/13784944/ECSE426_Microprocessor_Systems.pdf

Hardware: ARM STM32F4 discovery board.

Software: embedded C wiith CMSIS-RTOS library. 

Features:
- a real-time OS using threads and hardware interrupts. 
- sampling of a temperature sensor and an accelerometer, the later using DMA.
- 4 display modes using LEDs: temperature, orientation, acceleration direction, flashing.
- use of a push button to cycle through the four display modes.

For the final project, we implemented a Theremin-like music instrument, using:
- two boards
- external wireless modules to enable wireless communication between the boards
- a numerical keypad to control the instrument's pitch, hooked on the receiver.
- an LCD screen to display information such as volume, also hooked on the receiver.

Volume was dependant on the distance between the two boards, computed using the RSSI value included in wireless packets.

Timbre was dependant on the relative orientations between the two boards, computed by comparing data from the acceleromaters.


