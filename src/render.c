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
	int width, height;
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
	cairo_set_source_u32(cairo, state->bg);
	cairo_paint(cairo);

	PangoLayout *layout = pango_cairo_create_layout(cairo);
	PangoFontDescription *desc = pango_font_description_from_string(state->font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	cairo_set_source_u32(cairo, state->fg);
	pango_layout_set_text(layout, state->text, -1);
	pango_layout_get_size(layout, &width, &height);
	int x;
	if (state->right)
		x = state->width - width / PANGO_SCALE;
	else
		x = 0;
	cairo_move_to(cairo, x, (state->height - height / PANGO_SCALE) / 2);
	pango_cairo_show_layout(cairo, layout);
	g_object_unref(layout);
}

