#include "p8.h"
#include "input.h"

int input_x = 0;

bool input_jump = false;
int input_jump_pressed = 0;

bool input_grapple = false;
int input_grapple_pressed = 0;

int axis_x_value = 0;
bool axis_x_turned = false;

void update_input()
{
    // axes
    int prev_x = axis_x_value;
    if (P8_BTN(0) == CR_TRUE)
    {
        if (P8_BTN(1) == CR_TRUE)
        {
            if (axis_x_turned == true)
            {
                axis_x_value = prev_x;
                input_x = prev_x;
            }
            else 
            {
                axis_x_turned = true;
                axis_x_value = -prev_x;
                input_x = -prev_x;
            }
        }
        else 
        {
            axis_x_turned = false;
            axis_x_value = -1;
            input_x = -1;
        }
    }
    else if (P8_BTN(1) == CR_TRUE)
    {
        axis_x_turned = false;
        axis_x_value = 1;
        input_x = 1;
    }
    else 
    {
        axis_x_turned = false;
        axis_x_value = 0;
        input_x = 0;
    }

    // input jump
    bool jump = (P8_BTN(4) == CR_TRUE) ? true : false;
    if (jump == CR_TRUE && input_jump != false)
    {
        input_jump_pressed = 4;
    }
    else 
    {
        input_jump_pressed = (jump == true) ? P8_MAX_INT(0, input_jump_pressed - 1) : 0;
    }
    input_jump = jump;

    // input grapple
    bool grapple = (P8_BTN(5) == CR_TRUE) ? true : false;
    if (grapple == true && input_grapple != false)
    {
        input_grapple_pressed = 4;
    }
    else
    {
        input_grapple_pressed = (grapple == true) ? P8_MAX_INT(0, input_grapple_pressed - 1) : 0;
    }
    input_grapple = grapple;
}

bool consume_jump_press()
{
    bool val = (input_jump_pressed > 0) ? true : false;
    input_jump_pressed = 0;
    return val;
}

bool consume_grapple_press()
{
    bool val = (input_grapple_pressed > 0) ? true : false;
    input_grapple_pressed = 0;
    return val;
}