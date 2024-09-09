#include <unistd.h>
#include <sys/mman.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include "shm.h"
#include "state.h"
#include "wayland.h"
#include "render.h"


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


struct wl_buffer *
render(struct state *state)
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
			state->width, state->height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	cairo_surface_t *surface = cairo_image_surface_create_for_data(
	  data,
		CAIRO_FORMAT_ARGB32,
		state->width,
		state->height,
		stride
	);
	cairo_t *cairo = cairo_create(surface);

	cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
	cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
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
	g_object_unref(layout);

	munmap(data, size);
	wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
	return buffer;
}

