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
	uint32_t normal_bg, normal_fg, select_bg, select_fg;
	char *font;
	char *items[BUFSIZ];
	int item_count;
	int anchor;
};

struct state *state_init(int argc, char *argv[]);
void parse_input(struct state *state, char *input);

#endif
