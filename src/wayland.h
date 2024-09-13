#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"


struct wl_buffer *create_buffer(struct state *state);
void wayland_init(struct state *state);
  
#endif
