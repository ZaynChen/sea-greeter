#include <unistd.h>

#include "utils/jsc-utils.h"

static JSCContext *Context = NULL;

JSCContext *
get_global_context(void)
{
  if (Context == NULL)
    Context = jsc_context_new();
  return Context;
}

/**
 * Converts a JSCValue to a string
 */
gchar *
js_value_to_string_or_null(JSCValue *value)
{
  if (!jsc_value_is_string(value))
    return NULL;
  return jsc_value_to_string(value);
}

/**
 * Initialize the properties of a class
 */
void
initialize_class_properties(JSCClass *class, const struct JSCClassProperty properties[])
{
  int i = 0;
  struct JSCClassProperty current = properties[i];
  while (current.name != NULL) {
    switch (current.property_type) {
      case JSC_TYPE_VALUE_POST:
        current.property_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_property(class, current.name, current.property_type, current.getter, current.setter, NULL, NULL);
    i++;
    current = properties[i];
  }
}

/**
 * Initialize the properties of a class
 */
void
initialize_class_methods(JSCClass *class, const struct JSCClassMethod methods[])
{
  int i = 0;
  struct JSCClassMethod current = methods[i];
  while (current.name != NULL) {
    switch (current.return_type) {
      case JSC_TYPE_VALUE_POST:
        current.return_type = JSC_TYPE_VALUE;
        break;
    }
    jsc_class_add_method_variadic(class, current.name, current.callback, NULL, NULL, current.return_type);
    i++;
    current = methods[i];
  }
}

static void
jsc_g_ptr_array_free(gpointer data)
{
  g_object_unref(data);
}

GPtrArray *
jsc_array_to_g_ptr_array(JSCValue *jsc_array)
{
  if (!jsc_value_is_array(jsc_array)) {
    return NULL;
  }
  GPtrArray *array = g_ptr_array_new_with_free_func(jsc_g_ptr_array_free);
  JSCValue *jsc_array_length = jsc_value_object_get_property(jsc_array, "length");

  int length = jsc_value_to_int32(jsc_array_length);
  g_object_unref(jsc_array_length);

  for (int i = 0; i < length; i++) {
    g_ptr_array_add(array, jsc_value_object_get_property_at_index(jsc_array, i));
  }

  return array;
}

/**
 * Convert JSCValue parameters to GVariant
 * @param context The JSCContext
 * @param name Custom string to send, useful to execute a "name" method with given parameters
 * @param parameters A GPtrArray of JSCValue parameters
 */
GVariant *
jsc_parameters_to_g_variant_array(JSCContext *context, const gchar *name, GPtrArray *parameters)
{
  JSCValue *jsc_params;
  if (parameters == NULL) {
    jsc_params = jsc_value_new_array(context, G_TYPE_NONE);
  } else {
    jsc_params = jsc_value_new_array_from_garray(context, parameters);
  }
  char *json_params = jsc_value_to_json(jsc_params, 0);
  GVariant *name_p = g_variant_new_string(name);
  GVariant *params = g_variant_new_string(json_params);

  GVariant *param_arr[] = { name_p, params };

  GVariant *result = g_variant_new_array(G_VARIANT_TYPE_STRING, param_arr, G_N_ELEMENTS(param_arr));

  g_free(json_params);
  g_object_unref(jsc_params);
  return result;
}

JSCValue *
g_variant_reply_to_jsc_value(JSCContext *context, GVariant *reply)
{
  if (reply == NULL) {
    return NULL;
  }
  const gchar *json_value = g_variant_get_string(reply, NULL);
  JSCValue *value = jsc_value_new_from_json(context, json_value);
  if (jsc_value_is_null(value)) {
    return NULL;
  }
  return value;
}
