#ifndef MOTOR_MODES_H
#define MOTOR_MODES_H

#include <stdbool.h>
#include "ui/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void motor_modes_init(ui_mode_t initial_mode);
void motor_modes_set_mode(ui_mode_t mode);
void motor_modes_update(ui_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_MODES_H