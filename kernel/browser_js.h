#ifndef BROWSER_JS_H
#define BROWSER_JS_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void js_init(void);
int js_run(const char *source, int length);
void js_register_element(const char *tag, const char *id, int page_index);
int js_find_element_by_id(const char *id, char *tag_out, int tag_cap, int *page_index_out);
void js_dispatch_click(const char *element_id);

extern void js_host_alert(const char *msg);
extern void js_host_log(const char *msg);

#ifdef __cplusplus
}
#endif

#endif
