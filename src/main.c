#include <string.h>
#include <stdlib.h>

#include "state.h"
#include "wayland.h"

int
main(int argc, char *argv[])
{
	struct state *state = state_init(argc, argv);
	char input[BUFSIZ];
	while (wl_display_dispatch(state->wl_display)) {
		if (fgets(input, BUFSIZ, stdin)) {
			parse_input(state, input);
			wl_surface_attach(state->wl_surface, create_buffer(state), 0, 0);
			wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
			wl_surface_commit(state->wl_surface);
		}
	}

	return 0;
}
