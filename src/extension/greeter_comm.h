#ifndef EXTENSION_GREETER_COMM_H
#define EXTENSION_GREETER_COMM_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit-web-process-extension.h>

void GreeterComm_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebProcessExtension *extension);

#endif
