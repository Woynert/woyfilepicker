project('olivec', 'c')
#cc = meson.get_compiler('c')
#math_dep = cc.find_library('m', required : false)
olivec_includes = include_directories('./src')
olivec_lib = library('olivec',
  './olivec_implementation.c',
  include_directories: olivec_includes,
  #dependencies: math_dep,
  #install: true,
  #c_args: [ '-Wall', '-Wextra' ]
)
olivec_dep = declare_dependency(
  include_directories: olivec_includes,
  link_with: olivec_lib
)
