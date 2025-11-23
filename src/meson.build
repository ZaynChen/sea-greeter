configure_file(output: 'config.h', configuration: conf)

web_extension_sources = [
  'utils/ipc-renderer.c',
  'utils/jsc-utils.c',
  'utils/utils.c',

  'webext/extensions.c',
  'webext/lightdm-extension.c',

  'webext/extension/lightdm.c',
  'webext/extension/lightdm-signal.c',
  'webext/extension/greeter_config.c',
  'webext/extension/theme_utils.c',
  'webext/extension/greeter_comm.c',
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
  'utils/utils.c',
  'utils/jsc-utils.c',

  'greeter/main.c',
  'greeter/settings.c',
  'greeter/theme.c',

  'greeter/browser.c',
  'greeter/browser-web-view.c',
  'greeter/browser-commands.c',

  'greeter/bridge/lightdm.c',
  'greeter/bridge/greeter_config.c',
  'greeter/bridge/theme_utils.c',
  'greeter/bridge/greeter_comm.c',

  'greeter/bridge/bridge-object.c',
  'greeter/bridge/lightdm-objects.c',
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
