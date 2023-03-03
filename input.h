#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdbool.h>

extern int input_x;

extern bool input_jump;
extern int input_jump_pressed;

extern bool input_grapple;
extern int input_grapple_pressed;

extern int axis_x_value;
extern bool axis_x_turned;

extern void update_input();
extern bool consume_jump_press();
extern bool consume_grapple_press();

#endif