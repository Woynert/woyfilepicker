# DEBUG note: All of these debug options are set (albeit implicitely)
# in the Makefile. Check "make mesonSetupDebug".
# Compiler args:
#   -g
#   -fsanitize=address,undefined
#   -fno-omit-frame-pointer
#   -Og
# Linker args:
#   -fsanitize=address,undefined
#   -rdynamic   <-- NOT set by meson.

project('raylib-game', 'c', default_options : ['c_std=gnu11'])


# ↓↓↓ Includes function names for printing the stack.
add_project_link_arguments(['-rdynamic'], language:'c')
# ↑↑↑ @Note: Should I disable it on release? This can improve logs...


MY_WARNINGS = [
  '-Wall',
  '-Werror',
  '-Werror=incompatible-pointer-types',
  '-Wuninitialized',
  '-Wmaybe-uninitialized',
  '-Wmissing-field-initializers',

  '-Wextra',
  #'-Wpedantic',
  '-Wnull-dereference',
  # The following should be included in -Wextra, but including them anyway
  '-Wshadow',
  '-Wconversion',
  '-Werror=incompatible-pointer-types',
  #'-Wc++-compat',

  '-Wno-language-extension-token',  # Extension: (typeof).
  '-Wno-gnu-statement-expression',  # Extension: GNU statement expressions.
  #'-Wno-error=gnu-statement-expression',  # Extension: GNU statement expressions.


  '-Wno-type-limits',
  '-Wno-error=float-equal',
  '-Wno-error=unused-parameter',
  '-Wno-error=unused-function',
  '-Wno-error=unused-variable',
  '-Wno-error=unused-but-set-variable',
  '-Wno-unused-function',
  #-Wunused-function'
  '-Wno-error=stringop-truncation',

  #'-lundef=false', ## <-- TODO: Find out what this flag does.
  #'-Wno-unknown-warning-option'
]

#cc = meson.get_compiler('c')
#target_gcc_version = '10.0.0' 
#if cc.get_id() == 'gcc' and cc.version().version_compare('<' + target_gcc_version)
#else
  #MY_WARNINGS = MY_WARNINGS + [
  #'-Wno-error=stringop-truncation',
  #]
#endif



# strbuf

add_global_arguments('-DSTRBUF_PROVIDE_PRINTF', language: 'c')

# General dependencies

cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)

cwalk_proj = subproject('cwalk', default_options: [ 'default_library=shared' ])
cwalk_dep = cwalk_proj.get_variable('cwalk_dep')

mickstr_proj = subproject('mickstr', default_options: [ 'default_library=shared' ])
mickstr_dep = mickstr_proj.get_variable('mickstr_dep')

silk_proj = subproject('silk')
silk_dep = silk_proj.get_variable('silk_dep')

stb_proj = subproject('stb', default_options: [ 'default_library=shared' ])
stb_dep = stb_proj.get_variable('stb_dep')

olivec_dep = subproject('olivec', default_options: [ 'default_library=shared' ]).get_variable('olivec_dep')

cmake = import('cmake')
glfw_proj = cmake.subproject('glfw')
glfw_dep = glfw_proj.dependency('glfw')

woycontainer_proj = subproject('woycontainer')
woycontainer_dep = woycontainer_proj.get_variable('woycontainer_dep')

x11_dep = dependency('X11')
xext_dep = dependency('xext')

#gl_dep = dependency('gl') # Only needed to shut up clang LSP.
#dl_dep = dependency('dl')

# Artifacts

core_includes = include_directories('./src', './src/standalone')

exe = executable(
  'wpicker'
  ,'src/main.c'
  ,include_directories : core_includes
  ,dependencies : [
    glfw_dep,
    math_dep,
    mickstr_dep,
    woycontainer_dep,
    cwalk_dep,
    silk_dep,
    x11_dep,
    xext_dep,
    stb_dep,
    olivec_dep,
    #gl_dep,
    #dl_dep,
  ]
  ,c_args : MY_WARNINGS
)


