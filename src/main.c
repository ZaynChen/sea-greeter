#include <gtk/gtk.h>
#include <locale.h>
#include <stdlib.h>
#include <webkit/webkit.h>

#include "config.h"
#include "logger.h"
#include "settings.h"
#include "theme.h"

#include "bridge/greeter_comm.h"
#include "bridge/greeter_config.h"
#include "bridge/lightdm.h"
#include "bridge/theme_utils.h"
#include "browser.h"

#define PRIMARY_MONITOR 0

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <gdk/x11/gdkx.h>
#endif

static void
set_cursor(GdkDisplay *display)
{
#ifdef GDK_WINDOWING_X11
  if (GDK_IS_X11_DISPLAY(display)) {
    logger_debug("Setup root window cursor: GDK backend is X11");
    Window root_window = gdk_x11_display_get_xrootwindow(display);
    Display *xdisplay = gdk_x11_display_get_xdisplay(display);
    Cursor cursor = XCreateFontCursor(xdisplay, XC_left_ptr);
    XDefineCursor(xdisplay, root_window, cursor);
  }
#endif
}

extern GreeterConfig *greeter_config;

GPtrArray *greeter_browsers = NULL;

/*
 * Initialize web process extensions
 */
static void
initialize_web_process_extensions(WebKitWebContext *context, gpointer user_data)
{
  (void) user_data;

  gboolean secure_mode = greeter_config->greeter->secure_mode;
  gboolean detect_theme_errors = greeter_config->greeter->detect_theme_errors;
  g_autoptr(GVariant) data = NULL;
  data = g_variant_new("(bb)", secure_mode, detect_theme_errors);

  logger_debug("Extension initialized");

  webkit_web_context_set_web_process_extensions_directory(context, WEB_EXTENSIONS_DIR);
  webkit_web_context_set_web_process_extensions_initialization_user_data(context, g_steal_pointer(&data));
}

/*
 * Set keybinding accelerators
 */
static void
set_keybindings(GtkApplication *app)
{
  const struct accelerator {
    const gchar *action;
    const gchar *accelerators[9];
  } accels[] = {
    { "app.quit", { "<Control>Q", NULL } },

    { "win.toggle-inspector", { "<Shift><Primary>I", "F12", NULL } },

    { "win.undo", { "<Primary>Z", NULL } },
    { "win.redo", { "<Shift><Primary>Z", NULL } },

    { "win.copy", { "<Primary>C", NULL } },
    { "win.cut", { "<Primary>X", NULL } },
    { "win.paste", { "<Primary>V", NULL } },
    { "win.paste-plain", { "<Shift><Primary>V", NULL } },
    { "win.select-all", { "<Primary>A", NULL } },

    { "win.zoom-normal", { "<Primary>0", "<Primary>KP_0", NULL } },
    { "win.zoom-in", { "<Primary>plus", "<Primary>KP_Add", "<Primary>equal", "ZoomIn", NULL } },
    { "win.zoom-out", { "<Primary>minus", "<Primary>KP_Subtract", "ZoomOut", NULL } },
    { "win.fullscreen", { "F11", NULL } },

    { "win.reload", { "<Primary>R", "F5", "Refresh", "Reload", NULL } },
    { "win.force-reload", { "<Shift><Primary>R", "<Shift>F5", NULL } },

    { "win.close", { "<Primary>W", NULL } },
    { "win.minimize", { "<Primary>M", NULL } },

    { NULL, { NULL } },
  };

  int accel_count = G_N_ELEMENTS(accels);
  for (int i = 0; i < accel_count; i++) {
    if (accels[i].action == NULL)
      break;
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), accels[i].action, accels[i].accelerators);
  }
}

static void
print_info(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  (void) user_data;
  logger_debug("INFO");
}

/*
 * Quit application forcedly
 */
static void
app_quit_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action;
  (void) parameter;
  g_application_quit(user_data);
}

/*
 * Initialize app actions
 */
static void
initialize_actions(GtkApplication *app)
{
  static const GActionEntry app_entries[] = {
    { "info", print_info, NULL, NULL, NULL, { 0 } },
    { "quit", app_quit_cb, NULL, NULL, NULL, { 0 } },
  };

  g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
}
/*
 * Callback to be executed when app is activated.
 * Occurs after ":startup"
 */
static void
app_activate_cb(GtkApplication *app, gpointer user_data)
{
  (void) user_data;

  LightDM_initialize();
  GreeterConfig_initialize();
  ThemeUtils_initialize();
  GreeterComm_initialize();

  g_signal_connect(
      webkit_web_context_get_default(),
      "initialize-web-process-extensions",
      G_CALLBACK(initialize_web_process_extensions),
      NULL);

  greeter_browsers = g_ptr_array_new();

  GdkDisplay *display = gdk_display_get_default();
  GListModel *monitors = gdk_display_get_monitors(display);

  set_cursor(display);

  guint n_monitors = g_list_model_get_n_items(monitors);
  gboolean debug_mode = greeter_config->greeter->debug_mode;

  for (guint i = 0; i < n_monitors; i++) {
    GdkMonitor *monitor = g_list_model_get_item(monitors, i);
    gboolean is_primary = i == PRIMARY_MONITOR;
    Browser *browser = browser_new_full(app, monitor, debug_mode, is_primary);
    g_ptr_array_add(greeter_browsers, browser);

    load_theme(browser);
  }
  browser_set_overall_boundary(greeter_browsers);
}

/*
 * Callback to be executed when app is started.
 * Occurs before ":activate"
 */
static void
app_startup_cb(GtkApplication *app, gpointer user_data)
{
  (void) user_data;
  initialize_actions(app);
  set_keybindings(app);

  GtkBuilder *builder = gtk_builder_new_from_resource("/com/github/jezerm/sea_greeter/resources/menu_bar.ui");
  GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
  gtk_application_set_menubar(app, menu);

  g_object_unref(builder);
}

static void
g_application_parse_args(gint *argc, gchar ***argv)
{
  GOptionContext *context = g_option_context_new(NULL);

  gboolean version = false;
  gboolean api_version = false;

  gchar *mode_str = NULL;
  gboolean debug = false;
  gboolean normal = false;

  gchar *theme = NULL;
  gboolean list = false;

  GOptionEntry entries[] = {
    { "version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &version, "Version", NULL },
    { "api-version", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &api_version, "API version", NULL },

    { "mode", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &mode_str, "Mode", NULL },
    { "debug", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &debug, "Debug mode", NULL },
    { "normal", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &normal, "Normal mode", NULL },

    { "theme", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &theme, "Theme", NULL },
    { "list", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &list, "List installed themes", NULL },
    { NULL, 0, 0, 0, NULL, NULL, NULL },
  };

  g_option_context_add_main_entries(context, entries, NULL);
  GOptionGroup *option_group
      = g_option_group_new("sea-greeter", "sea-greeter description", "sea-greeter help_descritpion", NULL, NULL);
  g_option_context_add_group(context, option_group);
  g_option_context_set_help_enabled(context, true);

  g_option_context_parse(context, argc, argv, NULL);
  g_option_context_free(context);

  if (version) {
    printf("%s\n", VERSION);
    exit(0);
  }
  if (api_version) {
    printf("%s\n", API_VERSION);
    exit(0);
  }
  if (list) {
    print_themes();
    exit(0);
  }

  load_configuration();
  /*print_greeter_config();*/

  if (theme) {
    greeter_config->greeter->theme = g_strdup(theme);
    g_free(theme);
  }

  if (mode_str && debug && normal) {
    fprintf(stderr, "Conflict arguments: \"--mode\", \"--debug\" and \"--normal\"\n");
    exit(1);
  } else if (mode_str && debug) {
    fprintf(stderr, "Conflict arguments: \"--mode\" and \"--debug\"\n");
    exit(1);
  } else if (mode_str && normal) {
    fprintf(stderr, "Conflict arguments: \"--mode\" and \"--normal\"\n");
    exit(1);
  } else if (debug && normal) {
    fprintf(stderr, "Conflict arguments: \"--debug\" and \"--normal\"\n");
    exit(1);
  }

  if (mode_str && g_strcmp0(mode_str, "debug") == 0) {
    debug = true;
  } else if (mode_str && g_strcmp0(mode_str, "normal") == 0) {
    normal = true;
  } else if (mode_str) {
    fprintf(stderr, "Argument --mode should be: \"debug\" or \"normal\"\n");
    exit(1);
  }
  if (mode_str)
    g_free(mode_str);

  if (debug) {
    greeter_config->greeter->debug_mode = true;
  } else if (normal) {
    greeter_config->greeter->debug_mode = false;
  }

  load_theme_config();
}

int
main(int argc, char **argv)
{
  GtkApplication *app = gtk_application_new("com.github.jezerm.sea-greeter", G_APPLICATION_DEFAULT_FLAGS);

  setlocale(LC_ALL, "");

  WebKitApplicationInfo *web_info = webkit_application_info_new();
  webkit_application_info_set_name(web_info, "com.github.jezerm.sea-greeter");

  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_signal_connect(app, "startup", G_CALLBACK(app_startup_cb), NULL);

  g_application_parse_args(&argc, &argv);

  g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);
  webkit_application_info_unref(web_info);

  LightDM_destroy();
  GreeterConfig_destroy();
  ThemeUtils_destroy();
  GreeterComm_destroy();

  g_ptr_array_unref(greeter_browsers);

  free_greeter_config();
  logger_debug("Sea Greeter stopped");
  return 0;
}
