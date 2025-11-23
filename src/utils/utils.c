#include <glib.h>
#include <string.h>

/**
 * Get index position of *find* inside *source*
 */
int
string_get_index_of(const char *source, const char *find)
{
  int ind = -1;
  char *found = strstr(source, find);
  if (found != NULL) {
    ind = found - source;
  }
  return ind;
}

/**
 * Get index position of last ocurrence of *find* inside *source*
 */
int
string_get_last_index_of(const char *source, const char *find)
{
  if (source == NULL || find == NULL)
    return -1;
  int index = -1;

  char *source_rev = g_utf8_strreverse(source, -1);
  char *find_rev = g_utf8_strreverse(find, -1);

  int index_rev = string_get_index_of(source_rev, find_rev);
  if (index_rev != -1) {
    index = strlen(source) - index_rev;
  }

  g_free(source_rev);
  g_free(find_rev);
  return index;
}

const char *
g_variant_to_string(GVariant *variant)
{
  if (!g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
    return NULL;
  const gchar *value = g_variant_get_string(variant, NULL);
  return value;
}
