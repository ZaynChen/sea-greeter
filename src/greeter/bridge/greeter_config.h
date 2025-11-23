#ifndef BRIDGE_GREETER_CONFIG_H
#define BRIDGE_GREETER_CONFIG_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit.h>

#include "greeter/browser-web-view.h"

void GreeterConfig_destroy(void);
void GreeterConfig_initialize(void);
void handle_greeter_config_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
