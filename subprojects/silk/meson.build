#project('silk', 'c')
#silk_includes = include_directories('./silk')
#silk_dep = declare_dependency(
  #include_directories: silk_includes
#)


project('silk', 'c')
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
silk_includes = include_directories('./silk')
silk_lib = library('silk',
  './silk_implementation.c',
  dependencies: math_dep,
  include_directories: silk_includes,
  install: true,
  #c_args: [ '-Wall', '-Wextra' ]
)
silk_dep = declare_dependency(
  include_directories: silk_includes,
  link_with: silk_lib
)
