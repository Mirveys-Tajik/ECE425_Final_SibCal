# ECE425 Final Project Sib-Cal1



## Table of Contents  

[Introduction](#introduction)  

[Methodology](#methodology) 

[Block-Diagram](#Block-Diagram) 

[Results](#Results)  

[Components-list](#Components-list) 

[Pinout-Table](#Pinout-Table) 

[References](#References) 




<a name="Introduction"/>

## Introduction 
This project implements a four-function calculator using the TM4C123GH6PM LaunchPad and the EduBase training board, which provides an integrated 16×2 LCD and 4×4 matrix keypad.

The calculator supports:
 - Addition, subtraction, multiplication, and division
 - Decimal input (e.g., 12.3 + 3.7)
 - Chained operations (e.g., 1 + 2 = then + 4 =)
 - Real-time display of user input and results

All embedded software is written in C using Keil µVision and uses GPIO and SysTick peripherals for keypad scanning, LCD control, and timing.
    
Written by:
  - Mirveys Tajik

Professor:
  - Aaron Nanas

<a name="Methodology"/>

## Methodology
The calculator software is designed using a state-machine methodology consisting of three major states: entering the first number, entering the second number, and displaying the result. The keypad driver scans the matrix continuously by activating one column at a time and reading row inputs, while the LCD driver uses a 4-bit interface with Enable-pulse synchronization. Decimal numbers are parsed using the standard C function `strtod()`, allowing the calculator to support floating-point arithmetic. SysTick is used to generate accurate microsecond delays for LCD timing and key debouncing.

### Embedded concepts used
- GPIO  
  - Keypad scanning (PA2–PA5 → columns, PD0–PD3 → rows)  
  - LCD signaling (PA2–PA5 data, PC6 enable, PE0 RS)
- SysTick Timer  
  - Microsecond timing for LCD enable pulses  
  - Keypad debounce timing  
- State machine design  
  - ENTER_FIRST → ENTER_SECOND → SHOW_RESULT  
- Driver-based software organization  
  - EduBase_LCD.c  
  - Keypad.c  
  - SysTick_Delay.c  
  - main.c  

### Method
1. Continuous keypad scanning identifies key presses.  
2. Characters are appended to a text buffer (`entry[]`).  
3. When an operator is pressed, the first string converts to a number via `strtod()`.  
4. Second number is entered using the same buffer.  
5. Pressing `=` triggers the floating-point calculation.  
6. Result is formatted and displayed on LCD.



<a name="Results"/>

## Results & Demonstration
Testing confirmed reliable keypad entry, clean decimal handling, correct arithmetic operation, and stable LCD display updates. The calculator correctly displays ongoing expressions on the top LCD line and results on the bottom line. Edge cases such as divide-by-zero and long expression inputs were handled through error messages or truncation.

### 
![image]()


###  video
- [Demonstration Demo](https://youtube.com/shorts/IIVGqFhmd8g?feature=share)


<a name="Block-Diagram"/>

## Block-Diagram

![image]()

<a name="Components-list"/>

## Components-list

| Description                | Quantity      | Manufacture     | Purpose             |
| ---------------------------| ------------- |-----------------|---------------------|
| Tiva C Launchpad TM4C123G  | 1             | Texas Instrument|Main microcontroller |
| EduBase Board              | 1             | Trianer4Edu     |LCD + keypad hardware|
| USB-A to Micro-USB cable   | 1             | N/A             |   Power + debugging |                

<a name="Pinout-Table"/>

## Pinout-Table 

### Keypad Interface

| Function | Pin | Direction | Description |
|----------|------|-----------|-------------|
| Column 0 | PA2 | Output | Driven HIGH during scan |
| Column 1 | PA3 | Output | Driven HIGH during scan |
| Column 2 | PA4 | Output | Driven HIGH during scan |
| Column 3 | PA5 | Output | Driven HIGH during scan |
| Row 0 | PD0 | Input | Reads key press |
| Row 1 | PD1 | Input | Reads key press |
| Row 2 | PD2 | Input | Reads key press |
| Row 3 | PD3 | Input | Reads key press |

### LCD Pins
| Pin | Direction | Function |
|-----|-----------|----------|
| PA2–PA5 | Output | LCD Data (shared safely with keypad) |
| PE0 | Output | RS (Register Select) |
| PC6 | Output | E (Enable Pulse) |




<a name="References"/>

## References

- [Tiva C Series TM4C123G LaunchPad Evaluation Board User's Guide](https://www.ti.com/lit/pdf/spmu296)
- [Tiva TM4C123GH6PM Microcontroller Datasheet](https://www.ti.com/lit/gpn/TM4C123GH6PM)
- [EduBase V2 User Manual](https://www.ecs.csun.edu/~smirzaei/docs/ece425/EduBaseV2_user_guide_ver1.15.pdf)
