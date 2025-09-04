#include <webkit/webkit.h>

#include "browser-commands.h"
#include "browser-web-view.h"
#include "browser.h"

extern GPtrArray *greeter_browsers;

typedef struct {
  guint64 id;
} BrowserPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(Browser, browser, GTK_TYPE_APPLICATION_WINDOW)

typedef enum {
  PROP_ID = 1,
  PROP_MONITOR,
  PROP_DEBUG_MODE,
  PROP_IS_VALID,
  N_PROPERTIES,
} BrowserProperty;

static GParamSpec *browser_properties[N_PROPERTIES] = { NULL };

static void
browser_dispose(GObject *gobject)
{
  G_OBJECT_CLASS(browser_parent_class)->dispose(gobject);
}
static void
browser_finalize(GObject *gobject)
{
  // It is possible that object methods might be invoked
  // after dispose is run and before finalize runs
  //
  // Widgets in GTK 4 are treated like any other objects
  // - their parent widget holds a reference on them,
  // and GTK holds a reference on toplevel windows.
  // gtk_window_destroy() will drop the reference on the toplevel window,
  // and cause the whole widget hierarchy to be finalized
  // unless there are other references that keep widgets alive.
  gtk_window_destroy(GTK_WINDOW(gobject));
  g_ptr_array_remove(greeter_browsers, gobject);
  G_OBJECT_CLASS(browser_parent_class)->finalize(gobject);
}

static const GActionEntry win_entries[] = {
  { "undo", browser_undo_cb, NULL, NULL, NULL, { 0 } },
  { "redo", browser_redo_cb, NULL, NULL, NULL, { 0 } },

  { "copy", browser_copy_cb, NULL, NULL, NULL, { 0 } },
  { "cut", browser_cut_cb, NULL, NULL, NULL, { 0 } },
  { "paste", browser_paste_cb, NULL, NULL, NULL, { 0 } },
  { "paste-plain", browser_paste_plain_cb, NULL, NULL, NULL, { 0 } },
  { "select-all", browser_select_all_cb, NULL, NULL, NULL, { 0 } },

  { "zoom-normal", browser_zoom_normal_cb, NULL, NULL, NULL, { 0 } },
  { "zoom-in", browser_zoom_in_cb, NULL, NULL, NULL, { 0 } },
  { "zoom-out", browser_zoom_out_cb, NULL, NULL, NULL, { 0 } },

  { "reload", browser_reload_cb, NULL, NULL, NULL, { 0 } },
  { "force-reload", browser_force_reload_cb, NULL, NULL, NULL, { 0 } },
};
static const GActionEntry win_debug_entries[] = {
  { "toggle-inspector", browser_toggle_inspector_cb, NULL, NULL, NULL, { 0 } },

  { "fullscreen", browser_fullscreen_cb, NULL, NULL, NULL, { 0 } },

  { "close", browser_close_cb, NULL, NULL, NULL, { 0 } },
  { "minimize", browser_minimize_cb, NULL, NULL, NULL, { 0 } },
};

void
browser_show_menu_bar(Browser *browser, gboolean show)
{
  gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(browser), show);
}

static guint64
browser_gen_id(Browser *self)
{
  const char *manufacturer = gdk_monitor_get_manufacturer(self->monitor);
  const char *model = gdk_monitor_get_model(self->monitor);

  guint64 manufacturer_hash = manufacturer == NULL ? 0 : g_str_hash(manufacturer);
  guint64 model_hash = model == NULL ? 0 : g_str_hash(model);

  return (manufacturer_hash << 24) | (model_hash << 8);
}

static void
browser_initiate_metadata(Browser *self)
{
  BrowserPrivate *priv = browser_get_instance_private(self);

  self->meta.id = priv->id;
  self->meta.is_valid = self->is_valid;

  // a number of GtkWindow APIs that were X11-specific have been removed.
  // includes gtk_window_set_position()
  // Some windowing systems, such as Wayland, do not support a global coordinate system,
  // and thus the position of the window will always be (0, 0)
  // gtk_window_get_position(GTK_WINDOW(self), &self->meta.geometry.x, &self->meta.geometry.y);
  self->meta.geometry.x = 0;
  self->meta.geometry.y = 0;
  gtk_window_get_default_size(GTK_WINDOW(self), &self->meta.geometry.width, &self->meta.geometry.height);
}

void
browser_set_overall_boundary(GPtrArray *browsers)
{
  OverallBoundary overall_boundary;

  overall_boundary.maxX = -INT_MAX;
  overall_boundary.maxY = -INT_MAX;
  overall_boundary.minX = INT_MAX;
  overall_boundary.minY = INT_MAX;

  for (guint i = 0; i < browsers->len; i++) {
    Browser *browser = browsers->pdata[i];
    GdkRectangle geometry = browser->meta.geometry;
    overall_boundary.minX = MIN(overall_boundary.minX, geometry.x);
    overall_boundary.minY = MIN(overall_boundary.minY, geometry.y);
    overall_boundary.maxX = MAX(overall_boundary.maxX, geometry.x + geometry.width);
    overall_boundary.maxY = MAX(overall_boundary.maxY, geometry.y + geometry.height);
  }
  for (guint i = 0; i < browsers->len; i++) {
    Browser *browser = browsers->pdata[i];
    browser->meta.overall_boundary = overall_boundary;
  }
}

static void
browser_constructed(GObject *object)
{
  G_OBJECT_CLASS(browser_parent_class)->constructed(object);
  Browser *browser = BROWSER_WINDOW(object);
  BrowserPrivate *priv = browser_get_instance_private(browser);

  gtk_widget_set_cursor_from_name(GTK_WIDGET(object), "default");

  GdkRectangle geometry;
  gdk_monitor_get_geometry(browser->monitor, &geometry);

  gtk_window_set_default_size(GTK_WINDOW(browser), geometry.width, geometry.height);

  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(provider, "/com/github/jezerm/sea_greeter/resources/style.css");
  GdkDisplay *display = gdk_monitor_get_display(browser->monitor);
  gtk_style_context_add_provider_for_display(
      display,
      GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref(provider);

  g_action_map_add_action_entries(G_ACTION_MAP(browser), win_entries, G_N_ELEMENTS(win_entries), browser);

  priv->id = browser_gen_id(browser);

  browser_initiate_metadata(browser);

  if (browser->debug_mode) {
    g_action_map_add_action_entries(G_ACTION_MAP(browser), win_debug_entries, G_N_ELEMENTS(win_debug_entries), browser);
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(browser), true);
    browser_web_view_set_developer_tools(browser->web_view, true);
  } else {
    browser_web_view_set_developer_tools(browser->web_view, false);
    gtk_window_fullscreen(GTK_WINDOW(browser));
  }
}

static void
browser_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Browser *self = BROWSER_WINDOW(object);
  switch ((BrowserProperty) property_id) {
    case PROP_MONITOR:
      self->monitor = g_value_dup_object(value);
      break;
    case PROP_DEBUG_MODE:
      self->debug_mode = g_value_get_boolean(value);
      break;
    case PROP_IS_VALID:
      self->is_valid = g_value_get_boolean(value);
      break;
    default:
      break;
  }
}
static void
browser_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  (void) pspec;
  Browser *self = BROWSER_WINDOW(object);
  BrowserPrivate *priv = browser_get_instance_private(self);

  switch ((BrowserProperty) property_id) {
    case PROP_ID:
      g_value_set_int(value, priv->id);
      break;
    case PROP_MONITOR:
      g_value_set_object(value, self->monitor);
      break;
    case PROP_DEBUG_MODE:
      g_value_set_boolean(value, self->debug_mode);
      break;
    case PROP_IS_VALID:
      g_value_set_boolean(value, self->is_valid);
      break;
    default:
      break;
  }
}

static void
browser_class_init(BrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = browser_set_property;
  object_class->get_property = browser_get_property;

  object_class->dispose = browser_dispose;
  object_class->finalize = browser_finalize;
  object_class->constructed = browser_constructed;

  // TODO: remove browser destory
  browser_properties[PROP_ID] = g_param_spec_int("id", "ID", "The window internal id", 0, INT_MAX, 0, G_PARAM_READABLE);

  browser_properties[PROP_MONITOR] = g_param_spec_object(
      "monitor",
      "Monitor",
      "Monitor where browser should be placed",
      GDK_TYPE_MONITOR,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  browser_properties[PROP_DEBUG_MODE] = g_param_spec_boolean(
      "debug_mode",
      "DebugMode",
      "Whether the greeter is in debug mode or not",
      false,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  browser_properties[PROP_IS_VALID] = g_param_spec_boolean(
      "is_valid",
      "IsValid",
      "Whether the browser is in a valid monitor or not",
      true,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, browser_properties);
}

static void
browser_init(Browser *self)
{
  self->web_view = browser_web_view_new();
  self->is_valid = true;

  gtk_window_set_child(GTK_WINDOW(self), GTK_WIDGET(self->web_view));
}

Browser *
browser_new(GtkApplication *app, GdkMonitor *monitor)
{
  Browser *browser = g_object_new(BROWSER_TYPE, "application", app, "monitor", monitor, NULL);
  return browser;
}
Browser *
browser_new_full(GtkApplication *app, GdkMonitor *monitor, gboolean debug_mode, gboolean is_valid)
{
  Browser *browser = g_object_new(
      BROWSER_TYPE,
      "application",
      app,
      "monitor",
      monitor,
      "debug_mode",
      debug_mode,
      "is_valid",
      is_valid,
      NULL);
  return browser;
}
