
strbuf_extra_test = executable(
  'strbuf_extra_test'
  ,'./strbuf_extra_test.c'
  ,include_directories: core_includes
  ,dependencies: [
    mickstr_dep,
    woycontainer_dep
  ]
  ,c_args: MY_WARNINGS
  ,build_by_default: false
)
test('strbuf_extra_test', strbuf_extra_test)
