#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit-web-process-extension.h>

void ThemeUtils_initialize(WebKitWebPage *web_page, JSCContext *js_context);

#endif
