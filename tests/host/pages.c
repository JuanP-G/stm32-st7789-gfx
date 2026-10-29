/* Renders every demo and self-test page on the controller model.
 *
 *   pages hash            print one line per page: set, index, name, FNV-1a hash, bus writes
 *   pages render <dir>    write every page as <dir>/<set>_<index>.ppm
 *
 * The "hash" output is what tests/golden/pages.golden stores, and what the
 * multi-file and single-header builds are compared with. */
#include "sim.h"
#include "gfx_all.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	const char *set;
	int (*count)(void);
	const char *(*name)(int);
	void (*draw)(int);
} page_set_t;

static const page_set_t sets[] = {
	{ "demo",     gfx_demo_page_count,     gfx_demo_page_name,     gfx_demo_page },
	{ "selftest", gfx_selftest_page_count, gfx_selftest_page_name, gfx_selftest_page },
};

int main(int argc, char **argv)
{
	int render = (argc >= 3 && !strcmp(argv[1], "render"));
	if (!render && !(argc >= 2 && !strcmp(argv[1], "hash"))) {
		fprintf(stderr, "usage: %s hash | render <dir>\n", argv[0]);
		return 2;
	}
	for (unsigned s = 0; s < sizeof sets / sizeof sets[0]; s++) {
		for (int p = 0; p < sets[s].count(); p++) {
			sim_reset(); sim_fill_ram(0);
			sets[s].draw(p);
			if (sim_violations) {
				fprintf(stderr, "%s page %d: %" PRIu64 " window violations\n", sets[s].set, p, sim_violations);
				return 1;
			}
			if (render) {
				char path[512];
				snprintf(path, sizeof path, "%s/%s_%d.ppm", argv[2], sets[s].set, p);
				sim_save_ppm(path);
			} else {
				printf("%s %d \"%s\" %016" PRIx64 " %" PRIu64 "\n", sets[s].set, p, sets[s].name(p),
				       sim_hash_fnv1a(), sim_bus_writes());
			}
		}
	}
	return 0;
}
