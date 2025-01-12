#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include "state.h"
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


void
render(void *data, struct state *state)
{
	int *width_array = malloc(state->item_count * sizeof(int));
	int width = 0;
	int height;
	int i;

	int stride = state->width * 4;

	cairo_surface_t *surface = cairo_image_surface_create_for_data(
	  data,
		CAIRO_FORMAT_ARGB32,
		state->width,
		state->height,
		stride
	);
	cairo_t *cairo = cairo_create(surface);

	cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
	cairo_set_source_u32(cairo, state->normal_bg);
	cairo_paint(cairo);

	PangoLayout *layout = pango_cairo_create_layout(cairo);
	PangoFontDescription *desc = pango_font_description_from_string(state->font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	int j;
	for (i = 0; i < state->item_count; i++) {
		pango_layout_set_text(layout, state->items[i], -1);
		pango_layout_get_pixel_size(layout, &j, &height);
		width_array[i] = j;
		width += j;
	}

	int x;
	if (state->right)
		x = state->width - width;
	else
		x = 0;

	bool select = false;
	for (i = 0; i < state->item_count; i++) {
		pango_layout_set_text(layout, state->items[i], -1);
		if (select) {
			cairo_set_source_u32(cairo, state->select_bg);
			cairo_rectangle(cairo, x, 0, width_array[i], state->height);
			cairo_fill(cairo);
			cairo_set_source_u32(cairo, state->select_fg);
			select = false;
		} else {
			select = true;
			cairo_set_source_u32(cairo, state->normal_fg);
		}

		cairo_move_to(cairo, x, (state->height - height) / 2);
		pango_cairo_show_layout(cairo, layout);

		x += width_array[i];
	}

	g_object_unref(layout);
}

