configure_file(output: 'config.h', configuration: conf)

web_extension_sources = [
  'extensions.c',
  'lightdm-extension.c',
  'settings.c',
  'utils/ipc-renderer.c',
  'utils/utils.c',

  'extension/lightdm.c',
  'extension/lightdm-signal.c',
  'extension/greeter_config.c',
  'extension/theme_utils.c',
  'extension/greeter_comm.c',

  'bridge/lightdm-objects.c',
  'bridge/utils.c',
  #'bridge/lightdm-signal.c',
]

# gtk3 = dependency('gtk+-3.0', version: '>=3.24')
# webkit2 = dependency('webkit2gtk-4.0', version: '>=2.34')
# webkit2_webext = dependency('webkit2gtk-web-extension-4.0', version: '>=2.34')
gtk4 = dependency('gtk4', version: '>=4.18')
webkit = dependency('webkitgtk-6.0', version: '>=2.48')
webkit_webext = dependency('webkitgtk-web-process-extension-6.0', version: '>=2.48')
x11 = dependency('x11')

yaml = dependency('yaml-0.1', version: '>=0.2')
lightdm = dependency('liblightdm-gobject-1')

web_extension = library(
  'lightdm-sea-greeter-webext',
  web_extension_sources,
  dependencies: [webkit_webext, lightdm, yaml],
  install: true,
  install_dir: '/usr/lib/sea-greeter/',
)

pkg_mod = import('pkgconfig')
pkg_mod.generate(web_extension, description: 'webkit6-greeter-webext')

greeter_sources = [
  'main.c',
  'settings.c',
  'theme.c',

  'browser.c',
  'browser-web-view.c',
  'browser-commands.c',

  'utils/ipc-main.c',
  'utils/utils.c',

  'bridge/lightdm.c',
  'bridge/greeter_config.c',
  'bridge/theme_utils.c',
  'bridge/greeter_comm.c',

  'bridge/utils.c',
  'bridge/bridge-object.c',
  #'bridge/lightdm-signal.c',
  'bridge/lightdm-objects.c',
]

gnome = import('gnome')

greeter_sources += gnome.compile_resources('sea_greeter-resources', 'sea-greeter.gresource.xml', c_name: 'sea_greeter')

greeter = executable(
  'sea-greeter',
  greeter_sources,
  dependencies: [webkit, gtk4, yaml, lightdm, x11],
  install: true,
)

test('basic', greeter)
