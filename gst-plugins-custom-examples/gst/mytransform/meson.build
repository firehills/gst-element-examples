mytransform_sources = [
  'gstmytransform.c'
]

gstmytransform = library('gstmytransform',
  mytransform_sources,
  c_args : gst_plugins_bad_args,
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstaudio_dep, libm],
  install : true,
  install_dir : plugins_install_dir,
)
pkgconfig.generate(gstmytransform, install_dir : plugins_pkgconfig_install_dir)
plugins += [gstmytransform]
