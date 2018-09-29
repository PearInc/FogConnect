
add_executable(codec_test codec_test.c codec.c)
target_link_libraries(codec_test glib-2.0 event)
