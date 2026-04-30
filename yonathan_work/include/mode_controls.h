#ifndef MODE_CONTROLS_H
#define MODE_CONTROLS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mode_controls_init(void);
void mode_controls_update(void);
bool mode_controls_consume_advance_request(void);

#ifdef __cplusplus
}
#endif

#endif // MODE_CONTROLS_H
