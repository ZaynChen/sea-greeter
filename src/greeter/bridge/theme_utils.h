#ifndef BRIDGE_THEME_UTILS_H
#define BRIDGE_THEME_UTILS_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit.h>

#include "greeter/browser-web-view.h"

void ThemeUtils_initialize(void);
void ThemeUtils_destroy(void);
void handle_theme_utils_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
