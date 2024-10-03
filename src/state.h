#ifndef STATE_H
#define STATE_H

#include <stdio.h>
#include <stdbool.h>
#include <wayland-client.h>

struct state {
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_shm *wl_shm;
	struct wl_compositor *wl_compositor;
	struct wl_output *wl_output;
	struct wl_surface *wl_surface;
	struct zwlr_layer_shell_v1 *zwlr_layer_shell_v1;
	struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1;

	bool right;
	int width, height;
	uint32_t bg, fg;
	char *font;
	char text[BUFSIZ];
	int anchor;
};

struct state *state_init(int argc, char *argv[]);
bool state_update(struct state *state);

#endif
