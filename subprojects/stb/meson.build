project('stb', 'c')
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
stb_includes = include_directories('./src')
stb_lib = library('stb',
  './stb_implementation.c',
  include_directories: stb_includes,
  dependencies: math_dep,
  #install: true,
  #c_args: [ '-Wall', '-Wextra' ]
)
stb_dep = declare_dependency(
  include_directories: stb_includes,
  link_with: stb_lib
)
