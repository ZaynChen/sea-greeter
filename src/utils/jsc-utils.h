#ifndef JSC_UTILS_H
#define JSC_UTILS_H 1

#include <glib-object.h>
#include <jsc/jsc.h>

#define JSC_TYPE_VALUE_POST -1101

typedef struct _LDMObject {
  JSCContext *context;
  JSCValue *value;
} ldm_object;

struct JSCClassProperty {
  const gchar *name;
  GCallback getter;
  GCallback setter;
  GType property_type;
};
struct JSCClassMethod {
  const gchar *name;
  GCallback callback;
  GType return_type;
};
struct JSCClassSignal {
  const gchar *name;
};

JSCContext *get_global_context(void);

gchar *js_value_to_string_or_null(JSCValue *value);

void initialize_class_properties(JSCClass *class, const struct JSCClassProperty properties[]);

void initialize_class_methods(JSCClass *class, const struct JSCClassMethod methods[]);

void initialize_object_signals(JSCContext *js_context, JSCValue *object, const struct JSCClassSignal signals[]);

GPtrArray *jsc_array_to_g_ptr_array(JSCValue *jsc_array);

GVariant *jsc_parameters_to_g_variant_array(JSCContext *context, const gchar *name, GPtrArray *parameters);
JSCValue *g_variant_reply_to_jsc_value(JSCContext *context, GVariant *reply);

#endif
