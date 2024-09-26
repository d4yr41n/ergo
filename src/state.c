#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <pango/pangocairo.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include "state.h" 
#include "wayland.h" 

static int
get_font_height(const char *fontname)
{
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
parse_color(const char *color, uint32_t *result)
{
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

bool
state_update(struct state *state)
{
	int i;
	char c;
	bool update = false;
	for (i = 0; (c = getchar()) != EOF && i < BUFSIZ; i++) {
		if (!update) {
		  if (state->text[i] != c)
			  update = true;
		}
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

struct state *
state_init(int argc, char *argv[]) {
  struct state *state = malloc(sizeof(struct state));
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

  state->height = get_font_height(state->font) + 2;

  wayland_init(state);

  return state;
}
