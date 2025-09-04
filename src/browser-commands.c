#include <webkit/webkit.h>

#include "browser-commands.h"
#include "browser.h"

static float zoom_steps[] = {
  0.30f, 0.50f, 0.75f, 0.85f, 1.00f,

  1.15f, 1.25f, 1.50f, 1.75f,

  2.00f, 2.50f, 3.00f,
};

float
get_zoom_step_level(float current_level, int steps)
{
  float new_level = 0;
  int near_index = 0;
  int steps_count = G_N_ELEMENTS(zoom_steps);

  for (int i = 0; i < steps_count; i++) {
    if (current_level < zoom_steps[i])
      break;
    near_index = i;
  }
  int expected_index = near_index + steps;

  if (expected_index > steps_count - 1) {
    new_level = zoom_steps[steps_count - 1];
  } else if (expected_index <= 0) {
    new_level = zoom_steps[0];
  } else {
    new_level = zoom_steps[expected_index];
  }

  return new_level;
}

/*
 * Toggle web view inspector
 */
void
browser_toggle_inspector_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;

  BrowserWebView *web_view = browser->web_view;

  WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(web_view));
  WebKitWebViewBase *inspector_web_view = webkit_web_inspector_get_web_view(inspector);

  if (inspector_web_view != NULL) {
    webkit_web_inspector_close(WEBKIT_WEB_INSPECTOR(inspector));
  } else {
    webkit_web_inspector_show(WEBKIT_WEB_INSPECTOR(inspector));
  }
}

static void
browser_set_zoom(Browser *browser, int zoom)
{
  WebKitWebView *web_view = WEBKIT_WEB_VIEW(browser->web_view);
  double current_zoom = webkit_web_view_get_zoom_level(web_view);
  if (zoom == BROWSER_ZOOM_IN) {
    current_zoom = get_zoom_step_level(current_zoom, 1);
  } else if (zoom == BROWSER_ZOOM_OUT) {
    current_zoom = get_zoom_step_level(current_zoom, -1);
  }
  webkit_web_view_set_zoom_level(web_view, current_zoom);
}
void
browser_zoom_in_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  browser_set_zoom(browser, BROWSER_ZOOM_IN);
}
void
browser_zoom_out_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  browser_set_zoom(browser, BROWSER_ZOOM_OUT);
}
void
browser_zoom_normal_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(browser->web_view), 1.0f);
}

void
browser_undo_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;

  webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_UNDO);
}
void
browser_redo_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;

  webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_REDO);
}

static void
copy_text(GtkWidget *widget)
{
  GtkEditable *editable = GTK_EDITABLE(widget);

  // Initialize a GValue with the contents of the widget
  GValue value = G_VALUE_INIT;
  g_value_init(&value, G_TYPE_STRING);
  g_value_set_string(&value, gtk_editable_get_text(editable));

  // Store the value in the clipboard object
  GdkClipboard *clipboard = gtk_widget_get_clipboard(widget);
  gdk_clipboard_set_value(clipboard, &value);

  g_value_unset(&value);
}

void
browser_copy_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    copy_text(focused);
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_COPY);
  }
}

static void
cut_text(GtkWidget *widget)
{
  GtkEditable *editable = GTK_EDITABLE(widget);

  // Initialize a GValue with the contents of the widget
  GValue value = G_VALUE_INIT;
  g_value_init(&value, G_TYPE_STRING);
  g_value_set_string(&value, gtk_editable_get_text(editable));
  gtk_editable_set_text(editable, "");

  // Store the value in the clipboard object
  GdkClipboard *clipboard = gtk_widget_get_clipboard(widget);
  gdk_clipboard_set_value(clipboard, &value);

  g_value_unset(&value);
}

void
browser_cut_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    cut_text(focused);
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_CUT);
  }
}

static void
paste_text(GtkWidget *widget)
{
  GtkEditable *editable = GTK_EDITABLE(widget);

  // Initialize a GValue to receive text
  GValue value = G_VALUE_INIT;
  g_value_init(&value, G_TYPE_STRING);

  // Get the content provider for the clipboard, and ask it for text
  GdkClipboard *clipboard = gtk_widget_get_clipboard(widget);
  GdkContentProvider *provider = gdk_clipboard_get_content(clipboard);

  // If the content provider does not contain text, we are not interested
  if (!gdk_content_provider_get_value(provider, &value, NULL))
    return;

  const char *str = g_value_get_string(&value);

  gtk_editable_set_text(editable, str);

  g_value_unset(&value);
}

void
browser_paste_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    paste_text(focused);
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_PASTE);
  }
}
void
browser_paste_plain_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    paste_text(focused);
  } else {
    webkit_web_view_execute_editing_command(
        WEBKIT_WEB_VIEW(browser->web_view),
        WEBKIT_EDITING_COMMAND_PASTE_AS_PLAIN_TEXT);
  }
}
void
browser_select_all_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  GtkWidget *focused = gtk_window_get_focus(GTK_WINDOW(browser));

  if (GTK_IS_EDITABLE(focused)) {
    gtk_editable_select_region(GTK_EDITABLE(focused), 0, -1);
  } else {
    webkit_web_view_execute_editing_command(WEBKIT_WEB_VIEW(browser->web_view), WEBKIT_EDITING_COMMAND_SELECT_ALL);
  }
}

void
browser_reload_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  webkit_web_view_reload(WEBKIT_WEB_VIEW(browser->web_view));
}
void
browser_force_reload_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(browser->web_view));
}
void
browser_fullscreen_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;

  if (gtk_window_is_fullscreen(GTK_WINDOW(browser))) {
    gtk_window_unfullscreen(GTK_WINDOW(browser));
    browser_show_menu_bar(browser, true);
  } else {
    gtk_window_fullscreen(GTK_WINDOW(browser));
    browser_show_menu_bar(browser, false);
  }
}

void
browser_close_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  gtk_window_close(GTK_WINDOW(browser));
}
void
browser_minimize_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  Browser *browser = user_data;
  gtk_window_minimize(GTK_WINDOW(browser));
}
