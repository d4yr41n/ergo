#include <string.h>
#include <wayland-client.h>
#include <unistd.h>
#include <sys/mman.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include "state.h"
#include "render.h"
#include "wayland.h"
#include "shm.h"

void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
	wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release
};

static void
zwlr_layer_surface_v1_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
		uint32_t serial, uint32_t width, uint32_t height)
{
	struct state *state = data;
	state->width = width;
	state->height = height;
	zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);

	struct wl_buffer *buffer = create_buffer(state);
	wl_surface_attach(state->wl_surface, buffer, 0, 0);
	wl_surface_commit(state->wl_surface);
}

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener = {
	.configure = zwlr_layer_surface_v1_configure
};


static void
registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	struct state *state = data;
	if (strcmp(interface, wl_shm_interface.name) == 0) {
		state->wl_shm = wl_registry_bind(
			wl_registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, wl_compositor_interface.name) == 0) {
		state->wl_compositor = wl_registry_bind(
			wl_registry, name, &wl_compositor_interface, 4);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		state->zwlr_layer_shell_v1 = wl_registry_bind(
			wl_registry, name, &zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, wl_output_interface.name) == 0) {
		state->wl_output = wl_registry_bind(
			wl_registry, name, &wl_output_interface, 4);
	}
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = registry_global,
};

void
wayland_init(struct state *state)
{
	state->wl_display = wl_display_connect(NULL);
	state->wl_registry = wl_display_get_registry(state->wl_display);
	wl_registry_add_listener(state->wl_registry, &wl_registry_listener, state);
	wl_display_roundtrip(state->wl_display);

	state->wl_surface = wl_compositor_create_surface(state->wl_compositor);
	state->zwlr_layer_surface_v1 = zwlr_layer_shell_v1_get_layer_surface(
		state->zwlr_layer_shell_v1,
		state->wl_surface,
		state->wl_output,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP,
		"ergo"
	);
	zwlr_layer_surface_v1_set_anchor(state->zwlr_layer_surface_v1,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | state->anchor
	);
	zwlr_layer_surface_v1_set_size(state->zwlr_layer_surface_v1, state->width, state->height);
	zwlr_layer_surface_v1_set_exclusive_zone(state->zwlr_layer_surface_v1, state->height);
	zwlr_layer_surface_v1_add_listener(state->zwlr_layer_surface_v1, &zwlr_layer_surface_v1_listener, state);

	wl_surface_commit(state->wl_surface);
	wl_display_roundtrip(state->wl_display);
}

struct wl_buffer *
create_buffer(struct state *state)
{
	int stride = state->width * 4;
	int size = stride * state->height;

	int fd = allocate_shm_file(size);
	if (fd == -1) {
		return NULL;
	}

	// uint32_t 
	void *data = mmap(NULL, size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(state->wl_shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
			state->width, state->height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	render(data, state);

	munmap(data, size);
	wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
	return buffer;
}
