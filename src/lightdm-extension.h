#ifndef LIGHTDM_EXTENSION_H
#define LIGHTDM_EXTENSION_H 1

#include <glib-object.h>
#include <webkit/webkit-web-process-extension.h>

void web_page_initialize(WebKitWebProcessExtension *extension);

#endif
