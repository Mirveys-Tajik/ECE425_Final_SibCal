#ifndef EDUBASE_KEYPAD_H_
#define EDUBASE_KEYPAD_H_

#include <stdint.h>

/**
 * @brief Initialize the Edubase keypad GPIO pins.
 *        Rows are outputs, columns are inputs with pull-ups.
 */
void Keypad_Init(void);

/**
 * @brief Non-blocking read of keypad.
 * @return 0–15 for keys K0–K15, or -1 if no key is pressed.
 */
int Keypad_GetKeyIndex(void);

/**
 * @brief Non-blocking read, but returns the mapped character.
 * @return Mapped character (e.g. '1', '2', '+', etc.), or 0 if no key.
 */
char Keypad_GetChar(void);

/**
 * @brief Blocking read: wait until a key is pressed and debounced.
 * @return 0–15 for keys K0–K15.
 */
int Keypad_WaitForKeyIndex(void);

/**
 * @brief Blocking read: wait until a key is pressed and debounced.
 * @return Mapped character (never 0).
 */
char Keypad_WaitForChar(void);

#endif // EDUBASE_KEYPAD_H_
