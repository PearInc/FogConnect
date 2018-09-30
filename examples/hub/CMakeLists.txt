
add_executable(codec_test codec_test.c codec.c)
target_link_libraries(codec_test glib-2.0 event)

add_executable(hub hub.c codec.c)
target_link_libraries(hub glib-2.0 event fog_connection pthread event_pthreads)



