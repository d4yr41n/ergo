#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static void
randname(char *buf)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int
create_shm_file(void)
{
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		randname(name + sizeof(name) - 7);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}

static int
allocate_shm_file(size_t size)
{
	int fd = create_shm_file();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

struct client_state {
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_shm *wl_shm;
	struct wl_compositor *wl_compositor;
	struct wl_output *wl_output;
	struct wl_surface *wl_surface;
	struct zwlr_layer_shell_v1 *zwlr_layer_shell_v1;
	struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1;

	int width, height;
	uint32_t bg, fg;
	char *font;
	char text[BUFSIZ];
	int anchor;
};

static void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
	wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
	.release = wl_buffer_release,
};

static void
cairo_set_source_u32(cairo_t *cairo, uint32_t color) {
	cairo_set_source_rgba(
		cairo,
		(color >> 24 & 0xff) / 255.,
		(color >> 16 & 0xff) / 255.,
		(color >> 8 & 0xff) / 255.,
		(color >> 0 & 0xff) / 255.
	);
}

static struct wl_buffer *
draw_frame(struct client_state *state)
{
	int width, height;
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
			state->width, state->height, stride, WL_SHM_FORMAT_XRGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	cairo_surface_t *surface = cairo_image_surface_create_for_data(data,
		CAIRO_FORMAT_ARGB32, state->width, state->height, stride);
	cairo_t *cairo = cairo_create(surface);

	cairo_set_source_u32(cairo, state->bg);
	cairo_paint(cairo);

	PangoLayout *layout = pango_cairo_create_layout(cairo);
	PangoFontDescription *desc = pango_font_description_from_string(state->font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	cairo_set_source_u32(cairo, state->fg);
	pango_layout_set_text(layout, state->text, -1);
	pango_layout_get_size(layout, &width, &height);
	cairo_move_to(cairo, state->width - width / PANGO_SCALE,
		(state->height - height / PANGO_SCALE) / 2);
	pango_cairo_show_layout(cairo, layout);
	g_object_unref (layout);

	munmap(data, size);
	wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
	return buffer;
}

static void
zwlr_layer_surface_v1_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
		uint32_t serial, uint32_t width, uint32_t height)
{
	struct client_state *state = data;
	state->width = width;
	state->height = height;
	zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);

	struct wl_buffer *buffer = draw_frame(state);
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
	struct client_state *state = data;
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

static void
registry_global_remove(void *data,
		struct wl_registry *wl_registry, uint32_t name)
{
	/* This space deliberately left blank */
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

int
get_font_height(const char *fontname) {
	PangoFontMap *fontmap = pango_cairo_font_map_get_default();
	PangoContext *context = pango_font_map_create_context(fontmap);
	PangoFontDescription *desc = pango_font_description_from_string(fontname);
	PangoFont *font = pango_font_map_load_font(fontmap, context, desc);
	PangoFontMetrics *metrics = pango_font_get_metrics(font, NULL);
	int height = pango_font_metrics_get_height(metrics) / PANGO_SCALE;
	pango_font_metrics_unref(metrics);
	pango_font_description_free(desc);
	g_object_unref(context);
	g_object_unref(font);
	return height;
}


static bool
parse_color(const char *color, uint32_t *result) {
	if (color[0] == '#') {
		++color;
	}
	size_t len = strlen(color);
	if ((len != 6 && len != 8) || !isxdigit(color[0]) || !isxdigit(color[1])) {
		return false;
	}
	char *ptr;
	uint32_t parsed = (uint32_t)strtoul(color, &ptr, 16);
	if (*ptr != '\0') {
		return false;
	}
	*result = len == 6 ? ((parsed << 8) | 0xff) : parsed;
	return true;
}

static bool
read_stdin(struct client_state *state)
{
	int c, i;
	bool update = false;
	for (i = 0; (c = getchar()) != EOF && i < BUFSIZ; i++) {
		if (!update && state->text[i] != c)
			update = true;
		if (update) {
			if (c == '\n') {
				state->text[i] = '\0';
				break;
			}
			state->text[i] = c;
		}
	}
	return update;
}

int
main(int argc, char *argv[])
{
	struct client_state *state = calloc(1, sizeof(struct client_state));
	state->font = "monospace 10";
	state->bg = 0x000000ff;
	state->fg = 0xffffffff;
	state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
	const char *usage = "Usage: ergo [-B] [-F font] [-b color] [-f color]\n";
	int opt;
	while ((opt = getopt(argc, argv, "hBF:b:f:")) != -1) {
		switch (opt) {
			case 'B':
				state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
				break;
			case 'F':
				state->font = optarg;
				break;
			case 'b':
				if (!parse_color(optarg, &state->bg)) {
					fprintf(stderr, "Invalid background color: %s", optarg);
				}
				break;
			case 'f':
				if (!parse_color(optarg, &state->fg)) {
					fprintf(stderr, "Invalid foreground color: %s", optarg);
				}
				break;
			default:
				fprintf(stderr, "%s", usage);
				exit(EXIT_FAILURE);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "%s", usage);
		exit(EXIT_FAILURE);
	}

	state->height = get_font_height(state->font) + 3;

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

	while (wl_display_dispatch(state->wl_display)) {
		if (read_stdin(state)) {
			wl_surface_attach(state->wl_surface, draw_frame(state), 0, 0);
			wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
			wl_surface_commit(state->wl_surface);
		}
	}

	return 0;
}
