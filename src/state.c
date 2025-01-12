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
	g_object_unref(font);
	pango_font_description_free(desc);
	g_object_unref(context);
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

void parse_input(struct state *state, char *input) {
	int i = 0;
	char *ptr, *exc;
	state->items[i] = ptr = input;

	int length = strlen(input);
	if (input[length - 1] == '\n')
		input[length - 1] = '\0';

	while ((ptr = strchr(ptr, '^'))) {
		if ((exc = ptr - 1)[0] == '\\') {
		  memmove(exc, ptr, &input[length - 1] - exc);
		  length--;
		} else {
		  *ptr = '\0';
		  ptr++;
		  i++;
		  state->items[i] = ptr;
		}
	}
	state->item_count = i + 1;
}

struct state *
state_init(int argc, char *argv[]) {
  struct state *state = malloc(sizeof(struct state));
	state->font = "monospace 16";
	state->normal_bg = state->select_fg = 0x000000ff;
	state->normal_fg = state->select_bg = 0xffffffff;
	state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
	state->right = false;
	state->item_count = 0;
	const char *usage = "Usage: ergo [-br] [-f font] [-N color] [-n color] [-S color] [-s color]\n";
	int opt;
	while ((opt = getopt(argc, argv, "hbrf:N:n:S:s:")) != -1) {
		switch (opt) {
			case 'r':
				state->right = true;
				break;
			case 'b':
				state->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
				break;
			case 'f':
				state->font = optarg;
				break;
			case 'N':
				if (!parse_color(optarg, &state->normal_bg)) {
					fprintf(stderr, "Invalid normal background color: %s", optarg);
				}
				break;
			case 'n':
				if (!parse_color(optarg, &state->normal_fg)) {
					fprintf(stderr, "Invalid normal foreground color: %s", optarg);
				}
				break;
			case 'S':
				if (!parse_color(optarg, &state->select_bg)) {
					fprintf(stderr, "Invalid select background color: %s", optarg);
				}
				break;
			case 's':
				if (!parse_color(optarg, &state->select_fg)) {
					fprintf(stderr, "Invalid select foreground color: %s", optarg);
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
