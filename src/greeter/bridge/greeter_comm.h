#ifndef BRIDGE_GREETER_COMM_H
#define BRIDGE_GREETER_COMM_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit.h>

#include "greeter/browser-web-view.h"

void GreeterComm_initialize(void);
void GreeterComm_destroy(void);
void handle_greeter_comm_accessor(BrowserWebView *web_view, WebKitUserMessage *message);

#endif
