#ifndef EXTENSION_LIGHTDM_H
#define EXTENSION_LIGHTDM_H 1

#include <lightdm-gobject-1/lightdm.h>
#include <webkit/webkit-web-process-extension.h>

void LightDM_initialize(
    WebKitScriptWorld *world,
    WebKitWebPage *web_page,
    WebKitFrame *web_frame,
    WebKitWebProcessExtension *extension);

#endif
