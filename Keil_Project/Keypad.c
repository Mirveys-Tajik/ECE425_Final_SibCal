/**
 * @file Keypad.c
 *
 * @brief Driver for the EduBase 4x4 matrix keypad (K0–K15).
 *
 * Edubase keypad wiring:
 *   PA2 -> COL0
 *   PA3 -> COL1
 *   PA4 -> COL2
 *   PA5 -> COL3
 *
 *   PD0 -> ROW0
 *   PD1 -> ROW1
 *   PD2 -> ROW2
 *   PD3 -> ROW3
 *
 * Columns are outputs, rows are inputs.
 *
 * Key numbering (col-major):
 *   col0 (PA2): row0..3 => K0,  K1,  K2,  K3
 *   col1 (PA3): row0..3 => K4,  K5,  K6,  K7
 *   col2 (PA4): row0..3 => K8,  K9,  K10, K11
 *   col3 (PA5): row0..3 => K12, K13, K14, K15
 *
 * Mapping chosen to give layout:
 *
 *   7  8  9  /
 *   4  5  6  *
 *   1  2  3  -
 *   0  .  =  +
 */

#include "TM4C123GH6PM.h"
#include "Keypad.h"
#include "SysTick_Delay.h"
#include <stdint.h>

// ----- Pin mapping -----

// Columns on Port A: PA2–PA5 as outputs
#define COL_PORT       GPIOA
#define COL_MASK       0x3C      // bits 2–5 (PA2–PA5)
#define COL_SHIFT      2         // first column is PA2

// Rows on Port D: PD0–PD3 as inputs
#define ROW_PORT       GPIOD
#define ROW_MASK       0x0F      // bits 0–3 (PD0–PD3)
#define ROW_SHIFT      0         // first row is PD0

// Keypad mapping in column-major order.
// index = col * 4 + row
//
// Desired physical layout:
//
// Row0: 7   8   9   /
// Row1: 4   5   6   *
// Row2: 1   2   3   -
// Row3: 0   .   =   +
//
// Column-major fill:
// col0: [7,4,1,0]   -> K0,K1,K2,K3
// col1: [8,5,2,.]   -> K4,K5,K6,K7
// col2: [9,6,3,=]   -> K8,K9,K10,K11
// col3: [/,*, -,+]  -> K12,K13,K14,K15
static const char Keypad_Map[16] = {
    '7', '4', '1', '0',   // K0..K3
    '8', '5', '2', '.',   // K4..K7
    '9', '6', '3', '=',   // K8..K11
    '/', '*', '-', '+'    // K12..K15
};

/**
 * @brief Initialize the EduBase keypad GPIO pins.
 */
void Keypad_Init(void)
{
    // Enable Port A and Port D clocks
    SYSCTL->RCGCGPIO |= (1U << 0) | (1U << 3);  // A = bit 0, D = bit 3

    // Wait until the peripherals are ready
    while ((SYSCTL->PRGPIO & ((1U << 0) | (1U << 3))) == 0) {
        // spin
    }

    // ----- Configure columns (PA2–PA5) as outputs -----
    COL_PORT->DIR  |= COL_MASK;   // output
    COL_PORT->DEN  |= COL_MASK;   // digital enable

    // Start with all columns low (inactive)
    COL_PORT->DATA &= ~COL_MASK;

    // ----- Configure rows (PD0–PD3) as inputs -----
    ROW_PORT->DIR  &= ~ROW_MASK;  // input
    ROW_PORT->DEN  |= ROW_MASK;   // digital enable
    // No pull-ups/pull-downs needed; columns drive high when active
}

/**
 * @brief Scan the keypad once (internal helper, no debounce).
 * @return 0–15 for K0–K15 if a key is detected, or -1 if none.
 */
static int Keypad_ScanOnce(void)
{
    for (int col = 0; col < 4; col++)
    {
        // Drive all columns low
        uint32_t data = COL_PORT->DATA & ~COL_MASK;
        COL_PORT->DATA = data;

        // Drive only this column high (active)
        data |= (1U << (COL_SHIFT + col));
        COL_PORT->DATA = data;

        // Let signals settle
        SysTick_Delay1us(5);

        // Read rows. If a key in this column is pressed,
        // the corresponding row bit will be high.
        uint8_t rowData = (uint8_t)((ROW_PORT->DATA & ROW_MASK) >> ROW_SHIFT);

        if (rowData != 0)
        {
            // At least one key in this column is pressed
            for (int row = 0; row < 4; row++)
            {
                if (rowData & (1U << row))
                {
                    // Found (col, row)
                    return (col * 4) + row;  // K0..K15
                }
            }
        }
    }

    // No key pressed
    return -1;
}

/**
 * @brief Non-blocking read of the keypad with simple debounce.
 * @return 0–15 for K0–K15 if a stable key is detected, or -1 otherwise.
 */
int Keypad_GetKeyIndex(void)
{
    int first = Keypad_ScanOnce();
    if (first < 0)
        return -1;

    // Debounce: wait, then confirm it's still the same key
    SysTick_Delay1us(20000);   // ~20 ms
    int second = Keypad_ScanOnce();

    if (second == first)
        return first;

    return -1;
}

/**
 * @brief Non-blocking read of the keypad as a character.
 * @return mapped character (e.g. '1', '2', '+', etc.), or 0 if no key.
 */
char Keypad_GetChar(void)
{
    int idx = Keypad_GetKeyIndex();
    if (idx < 0)
        return 0;

    return Keypad_Map[idx];
}

/**
 * @brief Blocking read: wait until a key is pressed and released.
 * @return 0–15 index (K0–K15).
 */
int Keypad_WaitForKeyIndex(void)
{
    int key = -1;

    // Wait until a key press is detected
    while (key < 0)
    {
        key = Keypad_GetKeyIndex();
    }

    // Wait for key release to avoid auto-repeats
    int stillPressed = key;
    while (stillPressed >= 0)
    {
        stillPressed = Keypad_GetKeyIndex();
        SysTick_Delay1us(5000);   // small delay to avoid busy hammering
    }

    return key;
}

/**
 * @brief Blocking read: wait until a key is pressed and released.
 * @return mapped character.
 */
char Keypad_WaitForChar(void)
{
    int idx = Keypad_WaitForKeyIndex();
    return Keypad_Map[idx];
}
