
#include <string.h>
#include <alloca.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

#include "pgdb-internal.h"

struct destroy_info {
	const char	*dirname;
};

static bool destroy_iter(const struct dirent *de, void *priv, char **errptr)
{
	struct destroy_info *di = priv;
	size_t fn_len = strlen(di->dirname) + strlen(de->d_name) + 2;
	char *fn = alloca(fn_len);

	snprintf(fn, fn_len, "%s/%s", di->dirname, de->d_name);

	if (unlink(fn) < 0) {
		*errptr = strdup(strerror(errno));
		return false;	// stop dir iteration
	}

	return true;		// continue dir iteration
}

void pgdb_destroy_db(
    const pgdb_options_t* options,
    const char* name,
    char** errptr)
{
	// quick check: does superblock exist?
	if (!pg_have_superblock(name)) {
		*errptr = strdup("not a pgdb database");
		return;
	}

	// remove all files in that directory
	struct destroy_info di = { name };
	if (!pg_iterate_dir(name, destroy_iter, &di, errptr))
		return;

	// remove directory
	if (rmdir(name) < 0) {
		*errptr = strdup(strerror(errno));
		return;
	}

	*errptr = NULL;
}

