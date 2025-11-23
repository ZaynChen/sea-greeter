#include <webkit/webkit-web-process-extension.h>

#include "extension/greeter_comm.h"
#include "extension/greeter_config.h"
#include "extension/lightdm-signal.h"
#include "extension/lightdm.h"
#include "extension/theme_utils.h"

extern guint64 page_id;

static void
extension_initialize(WebKitScriptWorld *world, WebKitWebPage *web_page, WebKitFrame *web_frame)
{
  JSCContext *js_context = webkit_frame_get_js_context_for_script_world(web_frame, world);

  LightDM_signal_initialize(js_context);

  LightDM_initialize(web_page, js_context);
  GreeterConfig_initialize(web_page, js_context);
  ThemeUtils_initialize(web_page, js_context);
  GreeterComm_initialize(web_page, js_context);
}

void
web_page_initialize(void)
{
  g_signal_connect(webkit_script_world_get_default(), "window-object-cleared", G_CALLBACK(extension_initialize), NULL);
}
