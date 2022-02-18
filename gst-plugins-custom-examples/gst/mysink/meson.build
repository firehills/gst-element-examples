mysink_sources = [
  'gstmysink.c'
]

gstmysink = library('gstmysink',
  mysink_sources,
  c_args : gst_plugins_bad_args,
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstaudio_dep, libm],
  install : true,
  install_dir : plugins_install_dir,
)
pkgconfig.generate(gstmysink, install_dir : plugins_pkgconfig_install_dir)
plugins += [gstmysink]
