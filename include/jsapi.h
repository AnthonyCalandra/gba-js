#ifndef JSAPI_H
#define JSAPI_H 1

#include <stdbool.h>

bool initialize_jerry();
void cleanup_jerry();
void call_ontick_handler();

#endif
