/**
 * @file main.c
 *
 * @brief Main application for the TM4C123-based Sib-Cal project.
 *
 * This program implements a four-function calculator using the 
 * EduBase 16x2 LCD display and the EduBase 4x4 keypad. The user 
 * enters numbers and operators through the keypad, which are then
 * displayed on the LCD. The program processes input using a 
 * state-machine architecture consisting of:
 *
 *  - STATE_ENTER_FIRST:  User enters the first operand
 *  - STATE_ENTER_SECOND: User enters the second operand
 *  - STATE_SHOW_RESULT:  Final result displayed, supports chaining
 *
 * Decimal input is supported using string accumulation and floating-point
 * conversion via strtod(). The SysTick timer is used for keypad debounce
 * timing and LCD command delays.
 *
 * The program makes use of:
 *  - Keypad driver (Keypad.c/Keypad.h)
 *  - LCD driver (EduBase_LCD.c/EduBase_LCD.h)
 *  - SysTick delay driver (SysTick_Delay.c/SysTick_Delay.h)
 *
 * This file contains the main control loop, calculator logic, and
 * all display output routines required for the final ECE 425 project.
 *
 * @author Mirveys Tajik
 */


#include "TM4C123GH6PM.h"
#include "SysTick_Delay.h"
#include "EduBase_LCD.h"
#include "Keypad.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>   // for strtod

// Short LCD names
#define LCD_Init        EduBase_LCD_Init
#define LCD_Clear       EduBase_LCD_Clear_Display
#define LCD_SetCursor   EduBase_LCD_Set_Cursor
#define LCD_Print       EduBase_LCD_Display_String
#define LCD_SendChar    EduBase_LCD_Send_Data

typedef enum {
    STATE_ENTER_FIRST,
    STATE_ENTER_SECOND,
    STATE_SHOW_RESULT
} CalcState;

// Print a double compactly (fits within 16 chars)
static void LCD_PrintDoubleCompact(double x)
{
    char buf[17];
    // Up to 10 significant digits, total length <= 16
    snprintf(buf, sizeof(buf), "%.10g", x);
    LCD_Print((char*)buf);
}

// Show expression on the top line, e.g. "1.2+3.7="
static void update_expression_display(double op1, char op, double op2,
                                      uint8_t show_second, uint8_t show_equal)
{
    char buf[17];
    char temp[17];

    buf[0] = '\0';

    // op1
    snprintf(buf, sizeof(buf), "%.10g", op1);

    // operator
    if (op != 0)
    {
        size_t len = strlen(buf);
        if (len < sizeof(buf) - 1)
        {
            buf[len] = op;
            buf[len + 1] = '\0';
        }
    }

    // op2
    if (show_second)
    {
        snprintf(temp, sizeof(temp), "%.10g", op2);
        if (strlen(buf) + strlen(temp) < sizeof(buf))
        {
            strcat(buf, temp);
        }
    }

    // '='
    if (show_equal)
    {
        size_t len = strlen(buf);
        if (len < sizeof(buf) - 1)
        {
            buf[len] = '=';
            buf[len + 1] = '\0';
        }
    }

    LCD_SetCursor(0, 0);
    LCD_Print((char*)"                "); // clear top line
    LCD_SetCursor(0, 0);
    LCD_Print((char*)buf);
}

// Clear and show initial state
static void start_new_calculation(char *entry, size_t entry_size)
{
    LCD_Clear();
    memset(entry, 0, entry_size);

    LCD_SetCursor(0, 0);
    LCD_Print((char*)"Calc Ready");

    LCD_SetCursor(0, 1);
    LCD_Print((char*)"0");
    entry[0] = '0';
    entry[1] = '\0';
}

// Update bottom line from entry string
static void update_entry_display(const char *entry)
{
    LCD_SetCursor(0, 1);
    LCD_Print((char*)"                ");
    LCD_SetCursor(0, 1);
    LCD_Print((char*)entry);
}

int main(void)
{
    SysTick_Delay_Init();
    LCD_Init();
    Keypad_Init();

    CalcState state = STATE_ENTER_FIRST;

    double op1 = 0.0;
    double op2 = 0.0;
    double result = 0.0;
    char current_op = 0;

    // 16 chars max for LCD line, plus null terminator
    char entry[17] = {0};

    start_new_calculation(entry, sizeof(entry));

    while (1)
    {
        char key = Keypad_WaitForChar();

        if (state == STATE_ENTER_FIRST)
        {
            if ((key >= '0' && key <= '9') || key == '.')
            {
                // Only allow one '.' in the number
                if (key == '.' && strchr(entry, '.') != NULL)
                {
                    // ignore extra '.'
                }
                else
                {
                    // Handle leading zero (replace "0" with first digit/point)
                    if (strcmp(entry, "0") == 0 && key != '.')
                    {
                        entry[0] = '\0';
                    }

                    size_t len = strlen(entry);
                    if (len < sizeof(entry) - 1)   // limit to 16 chars
                    {
                        entry[len] = key;
                        entry[len + 1] = '\0';
                    }
                }

                update_entry_display(entry);
            }
            else if (key == '+' || key == '-' || key == '*' || key == '/')
            {
                // Convert entry to first operand (may have decimal)
                op1 = strtod(entry, NULL);
                current_op = key;
                state = STATE_ENTER_SECOND;

                // Show "op1 op" on top
                update_expression_display(op1, current_op, 0.0, 0, 0);

                // Prepare entry for second operand
                memset(entry, 0, sizeof(entry));
                entry[0] = '0';
                entry[1] = '\0';
                update_entry_display(entry);
            }
            else if (key == '=')
            {
                // '=' pressed without operator: just show entry as result
                op1 = strtod(entry, NULL);
                result = op1;
                state = STATE_SHOW_RESULT;
                current_op = 0;

                // Clear and show result only (no "Result:" text)
                LCD_Clear();
                LCD_SetCursor(0, 1);
                LCD_PrintDoubleCompact(result);
            }
        }
        else if (state == STATE_ENTER_SECOND)
        {
            if ((key >= '0' && key <= '9') || key == '.')
            {
                // Only one '.' allowed in second operand
                if (key == '.' && strchr(entry, '.') != NULL)
                {
                    // ignore extra '.'
                }
                else
                {
                    if (strcmp(entry, "0") == 0 && key != '.')
                    {
                        entry[0] = '\0';
                    }

                    size_t len = strlen(entry);
                    if (len < sizeof(entry) - 1)   // limit to 16 chars
                    {
                        entry[len] = key;
                        entry[len + 1] = '\0';
                    }
                }

                update_entry_display(entry);
            }
            else if (key == '=')
            {
                // Finalize second operand
                op2 = strtod(entry, NULL);

                // Show full expression "op1 op op2 =" on top
                update_expression_display(op1, current_op, op2, 1, 1);

                // Compute result
                if (current_op == '+')
                {
                    result = op1 + op2;
                }
                else if (current_op == '-')
                {
                    result = op1 - op2;
                }
                else if (current_op == '*')
                {
                    result = op1 * op2;
                }
                else if (current_op == '/')
                {
                    if (op2 == 0.0)
                    {
                        LCD_SetCursor(0, 0);
                        LCD_Print((char*)"Err: Div by 0  ");
                        LCD_SetCursor(0, 1);
                        LCD_Print((char*)"Press any key  ");
                        state = STATE_SHOW_RESULT;
                        continue;
                    }
                    else
                    {
                        result = op1 / op2;
                    }
                }

                // Bottom line: only the result (no label), up to 16 chars
                LCD_SetCursor(0, 1);
                LCD_Print((char*)"                ");
                LCD_SetCursor(0, 1);
                LCD_PrintDoubleCompact(result);

                state = STATE_SHOW_RESULT;
            }
            else if (key == '+' || key == '-' || key == '*' || key == '/')
            {
                // Change operator before entering second operand
                current_op = key;
                update_expression_display(op1, current_op, 0.0, 0, 0);
            }
        }
        else if (state == STATE_SHOW_RESULT)
        {
            if ((key >= '0' && key <= '9') || key == '.')
            {
                // Start a new calculation with fresh entry
                state = STATE_ENTER_FIRST;
                op1 = op2 = 0.0;
                current_op = 0;

                memset(entry, 0, sizeof(entry));
                entry[0] = key;
                entry[1] = '\0';

                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print((char*)"Calc Ready");
                update_entry_display(entry);
            }
            else if (key == '+' || key == '-' || key == '*' || key == '/')
            {
                // Chain: use last result as new op1
                op1 = result;
                op2 = 0.0;
                current_op = key;
                state = STATE_ENTER_SECOND;

                update_expression_display(op1, current_op, 0.0, 0, 0);

                memset(entry, 0, sizeof(entry));
                entry[0] = '0';
                entry[1] = '\0';
                update_entry_display(entry);
            }
            else if (key == '=')
            {
                // Could repeat last op, but do nothing for now
            }
        }
    }
}
