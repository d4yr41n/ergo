#ifndef WAYLAND_H
#define WAYLAND_H

#include "state.h"

void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer);

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release
};

void wayland_init(struct state *state);
  
#endif
