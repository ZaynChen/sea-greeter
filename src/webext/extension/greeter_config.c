#include <stdlib.h>
#include <unistd.h>

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit-web-process-extension.h>

#include "utils/jsc-utils.h"

#include "utils/ipc-renderer.h"

static WebKitWebPage *WebPage;
static JSCClass *GreeterConfig_class;
static ldm_object *GreeterConfig_object;

static JSCValue *
GreeterConfig_branding_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "greeter_config", "branding", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
GreeterConfig_greeter_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "greeter_config", "greeter", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
GreeterConfig_features_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "greeter_config", "features", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}
static JSCValue *
GreeterConfig_layouts_getter_cb(ldm_object *instance)
{
  JSCContext *context = instance->context;
  WebKitUserMessage *reply
      = ipc_renderer_send_message_sync_with_arguments(WebPage, context, "greeter_config", "layouts", NULL);
  if (reply == NULL) {
    return NULL;
  }
  GVariant *reply_param = webkit_user_message_get_parameters(reply);
  JSCValue *value = g_variant_reply_to_jsc_value(context, reply_param);

  return value;
}

static JSCValue *
GreeterConfig_constructor(JSCContext *context)
{
  return jsc_value_new_null(context);
}

void
GreeterConfig_initialize(WebKitWebPage *web_page, JSCContext *js_context)
{
  WebPage = web_page;
  JSCValue *global_object = jsc_context_get_global_object(js_context);

  if (GreeterConfig_object != NULL) {
    jsc_value_object_set_property(global_object, "greeter_config", GreeterConfig_object->value);
    return;
  }

  GreeterConfig_class = jsc_context_register_class(js_context, "__GreeterConfig", NULL, NULL, NULL);
  JSCValue *gc_constructor = jsc_class_add_constructor(
      GreeterConfig_class,
      NULL,
      G_CALLBACK(GreeterConfig_constructor),
      js_context,
      NULL,
      JSC_TYPE_VALUE,
      0,
      NULL);

  const struct JSCClassProperty GreeterConfig_properties[] = {
    { "branding", G_CALLBACK(GreeterConfig_branding_getter_cb), NULL, JSC_TYPE_VALUE },
    { "greeter", G_CALLBACK(GreeterConfig_greeter_getter_cb), NULL, JSC_TYPE_VALUE },
    { "features", G_CALLBACK(GreeterConfig_features_getter_cb), NULL, JSC_TYPE_VALUE },
    { "layouts", G_CALLBACK(GreeterConfig_layouts_getter_cb), NULL, JSC_TYPE_VALUE },
    { NULL, NULL, NULL, 0 },
  };

  initialize_class_properties(GreeterConfig_class, GreeterConfig_properties);

  JSCValue *value = jsc_value_constructor_callv(gc_constructor, 0, NULL);
  GreeterConfig_object = malloc(sizeof *GreeterConfig_object);
  GreeterConfig_object->value = value;
  GreeterConfig_object->context = js_context;

  JSCValue *greeter_config_object = jsc_value_new_object(js_context, GreeterConfig_object, GreeterConfig_class);
  GreeterConfig_object->value = greeter_config_object;

  jsc_value_object_set_property(global_object, "greeter_config", greeter_config_object);
}
