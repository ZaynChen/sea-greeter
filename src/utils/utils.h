#ifndef UTILS_H
#define UTILS_H

#include <glib.h>

int string_get_index_of(const char *source, const char *find);

int string_get_last_index_of(const char *source, const char *find);

const char *g_variant_to_string(GVariant *variant);

#endif
