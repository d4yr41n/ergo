#include "state.h"
#include "render.h"

int
main(int argc, char *argv[])
{
	struct state *state = state_init(argc, argv);

	while (wl_display_dispatch(state->wl_display)) {
		if (state_update(state)) {
			wl_surface_attach(state->wl_surface, render(state), 0, 0);
			wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
			wl_surface_commit(state->wl_surface);
		}
	}

	return 0;
}
