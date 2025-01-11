#ifndef UI_H
#define UI_H

void ui_display(float speed, int rpm, float distance, int cruise_enabled, int set_speed, int headlights);
char ui_get_user_command();
void ui_process_command(char command, int *cruise_enabled, int *set_speed, int *headlights);

#endif
