#ifndef PIXA3848_H
#define PIXA3848_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int pixa3848_init(void);
int pixa3848_poll(void);
int pixa3848_is_ready(void);
int pixa3848_shutdown(void);
int pixa3848_enable(void);
int pixa3848_disable(void);
int pixa3848_get_touch_count(void);
int pixa3848_get_finger(int index, int *x, int *y, int *pressure, int *width, int *height, int *touch_id, int *state);
int pixa3848_get_cursor(int *x, int *y);
int pixa3848_gesture_tap(void);
int pixa3848_gesture_double_tap(void);
int pixa3848_gesture_scroll(int *dx, int *dy);
int pixa3848_gesture_pinch(int *scale);
int pixa3848_gesture_swipe3(int *dx, int *dy);
int pixa3848_gesture_edge(int *edge);
int pixa3848_set_sensitivity(uint8_t level);
struct pixa3848_stats {
    uint32_t touches_detected;
    uint32_t taps;
    uint32_t double_taps;
    uint32_t scrolls;
    uint32_t pinches;
    uint32_t swipes3;
    uint32_t edge_gestures;
    uint32_t palm_rejections;
    uint32_t errors;
};
int pixa3848_stats_get(struct pixa3848_stats *out);
void pixa3848_stats_reset(void);

#ifdef __cplusplus
}
#endif

#endif
