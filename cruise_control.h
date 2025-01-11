#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

void cruise_control_enable(int speed);
void cruise_control_disable();
void cruise_control_adjust_speed(int delta);
void cruise_control_loop();

#endif // CRUISE_CONTROL_H
